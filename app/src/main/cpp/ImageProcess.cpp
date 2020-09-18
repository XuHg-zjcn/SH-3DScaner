//
// Created by xrj on 20-9-7.
//


#include "ImageProcess.h"
#include<android/bitmap.h>
#include <ctgmath>
#include <cstdint>
#include <semaphore.h>

#define u8_max 255
uint32_t n_thread;
sem_t sem;                //thread semaphore
volatile uint32_t thread_remain;
bool threads_running;      //set false to stop threads
pthread_t *threads;
array2d<uint8_t> *img_in;   //camera image input
pthread_mutex_t mutex;

point_u32::point_u32(uint32_t set_x, uint32_t set_y): xy<uint32_t>
{set_x<<16U,set_y<<16U}{}

point_u32::point_u32(point_u32 *old):xy<uint32_t>
{old->x, old->y}{}

inline void point_u32::add_delta(xy<int> *delta) {
    x += delta->x;
    y += delta->y;
}

inline void point_u32::update_H16_BL8(H16_BL8 *upd) {
    upd->H16.x = x>>16U;
    upd->H16.y = y>>16U;
    upd->BL8.x = (x>>8U)&0xffU;
    upd->BL8.y = (y>>8U)&0xffU;
}

//双线性插值 output 16bit 0-65536
uint32_t BiLinear(uint64_t *pNear4, uint32_t x, uint32_t y) {
    uint32_t sum=0;
    uint32_t ix = u8_max - x;
    uint32_t iy = u8_max - y;
    sum += *pNear4++ * ix * iy;
    sum += *pNear4++ * ix * y;
    sum += *pNear4++ * x * iy;
    sum += *pNear4   * x * y;
    return sum;
}
//获取用于双线性插值的四个点
void get_near4(array2d<uint8_t> *img, xy<uint32_t> *pH16, uint64_t *pNear4) {
    *pNear4++ = *img->get(pH16->y+0,pH16->x+0);
    *pNear4++ = *img->get(pH16->y+0,pH16->x+1);
    *pNear4++ = *img->get(pH16->y+1,pH16->x+0);
    *pNear4   = *img->get(pH16->y+1,pH16->x+1);
}
//N<256
uint32_t line_sum(array2d<uint8_t> *img, point_u32 *start, xy<int> *delta, uint32_t N) {
    point_u32 point = point_u32(start);
    H16_BL8 xy_part;
    uint32_t sum_value=0;
    uint64_t near4[4]; //test: u64 fast than u32
    for(uint32_t i=0;i<N;i++) {
        point.update_H16_BL8(&xy_part);
        get_near4(img, &xy_part.H16, near4);
        sum_value+=BiLinear(near4, xy_part.BL8.x, xy_part.BL8.y);
        point.add_delta(delta);
    }
    sum_value = (sum_value/N)>>16U;
    assert(sum_value<=0xff);
    return sum_value;
}

/* 霍夫变换
 *   ----> delta_point
 * |------
 * |------
 * |------
 * v
 * delta_line
 */
void lines_search(line_search_para &para, array2d<uint8_t> *img)
{
    float rad = para.rads.start;
    point_u32 point = point_u32(0, 0);
    xy<int> delta_point{0,0};//线上点的间距
    xy<int> delta_line{0,0}; //平行线的间距
    rad += para.rads.step * (para.rads).range.a;
    for(int i=(para.rads).range.a;i<(para.rads).range.b;i++) {
        delta_point.x = (int)(cos(rad)*0x10000);
        delta_point.y = (int)(sin(rad)*0x10000);
        delta_line.x = -delta_point.y;
        delta_line.y = delta_point.x;

        point = point_u32(para.start);
        uint8_t *ptr = para.out->get(i,0);
        for(int j=(para.N_line).a;j<(para.N_line).b;j++) {
            *ptr++ = line_sum(img, &point, &delta_point, para.N_length);
            point.add_delta(&delta_line);
        }

        rad += para.rads.step;
    }
}

void* thread_lines_search(void* args){
    auto *para = (line_search_para*) args;
    while(threads_running) {
        sem_wait(&sem);
        lines_search(*para, img_in);
        thread_remain--;
        if(thread_remain==0){
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(nullptr);
}

void init(int N_thread, line_search_para *paras, uint32_t in_width, uint32_t in_height) {
    uint32_t range_per_thread;
    pthread_attr_t attr;
    size_t stacksize;                  //修改栈大小=100kB
    line_search_para *threads_paras;   //每个线程的参数

    assert(N_thread>0);                //0<线程数<=50
    assert(N_thread<=50);
    n_thread = N_thread;

    thread_remain = N_thread;
    threads_running = true;
    img_in = new array2d<uint8_t>(in_width, in_height, false);
    sem_init(&sem, 0, 0); //pthread初始化
    pthread_mutex_init(&mutex, NULL);
    threads = (pthread_t*)malloc(sizeof(pthread_t)*N_thread);
    pthread_attr_init(&attr);
    pthread_attr_getstacksize(&attr, &stacksize);
    stacksize = 100000; //100KB
    pthread_attr_setstacksize(&attr, stacksize);
    threads_paras = (line_search_para*)malloc(sizeof(line_search_para)*N_thread);

    int_range &range=paras->rads.range;
    assert(range.b % N_thread == 0);
    range_per_thread = range.b / N_thread;
    range.b = range_per_thread;

    for(int i=0;i<N_thread;i++) {
        if(i==0){
            memcpy(threads_paras, paras, sizeof(line_search_para));
        }
        else{
            memcpy(threads_paras, threads_paras-1, sizeof(line_search_para));
        }
        pthread_create(threads, &attr, thread_lines_search, threads_paras);
        threads_paras->rads.range.a += range_per_thread; //a=range_per_thread*(N_thread)
        threads_paras->rads.range.b += range_per_thread; //b=range_per_thread*(N_thread+1)
        threads++;
        threads_paras++;
    }
}

int update(uint8_t *img_ptr) {
  thread_remain=n_thread;
  img_in->set_ptr(img_ptr);
  pthread_mutex_lock(&mutex);
  for(int i=0;i<n_thread;i++) {
      sem_post(&sem);
  }
  return 0;
}
