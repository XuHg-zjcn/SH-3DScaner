/*
 * Copyright 2020 Xu Ruijun
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <jni.h>
#include <string>
#include <pthread.h>
#include "ImageProcess.h"
#include "OptFlow.h"
#include "optflow_FFT.h"
#include <android/bitmap.h>
#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <ctime>
using namespace cv;
using namespace std;
array2d<uint8_t> *img; //相机照片
array2d<uint8_t> *tmp; //lines_search结果
array2d<uint32_t> *bmp;    //输出窗口
uint32_t *bmp_ptr;
line_search_para *ls_para;
int N_frames=0;   //processed frames已处理帧数
OptFlow *optflow;
optflow_FFT *OF_fft;
Point2i point_int[4];
bool AB=true;
uint32_t p_fft_bmp;
Mat small;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_update(JNIEnv *env, jclass thiz, jobject img_in) {
    //jlong Buf_Cap = env->GetDirectBufferCapacity(img_in);
    //assert(img->prod() == Buf_Cap);
    auto *in_ptr = (uint8_t*)env->GetDirectBufferAddress(img_in);
    if(update(in_ptr) != 0) //BUSY
        return;
    // TODO: too slow, want change to use OpenGL
    /*pthread_mutex_lock(&mutex);
    uint32_t *bmp_ptr_tmp = bmp->data;
    uint8_t *tmp_ptr = tmp->data;
    uint64_t prod = bmp->prod();
    for(uint64_t i=0; i<prod; i++) {
        *bmp_ptr_tmp++ = (*tmp_ptr++)*0x00010101 + 0xff000000;
    }
    pthread_mutex_unlock(&mutex);*/
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_jni_1init(JNIEnv *env, jclass thiz, jobject para,
                                                   jobject bmp_out) {
    jfieldID id;
    jclass c = env->FindClass("com/example/sh3dscaner/ImageProcess$init_para");
    id = env->GetFieldID(c, "in_width", "I");
    jint in_width = env->GetIntField(para, id);
    id = env->GetFieldID(c, "in_height", "I");
    jint in_height = env->GetIntField(para, id);
    id = env->GetFieldID(c, "start_x", "I");
    jint start_x = env->GetIntField(para, id);
    id = env->GetFieldID(c, "start_y", "I");
    jint start_y = env->GetIntField(para, id);
    id = env->GetFieldID(c, "rad_start", "F");
    jfloat rad_start = env->GetFloatField(para, id);
    id = env->GetFieldID(c, "rad_step", "F");
    jfloat rad_step = env->GetFloatField(para, id);
    id = env->GetFieldID(c, "rad_N", "I");
    jint rad_N = env->GetIntField(para, id);
    id = env->GetFieldID(c, "N_line", "I");
    jint n_line = env->GetIntField(para, id);
    id = env->GetFieldID(c, "N_length", "I");
    jint n_length = env->GetIntField(para, id);
    id = env->GetFieldID(c, "N_thread", "I");
    jint n_thread = env->GetIntField(para, id);
    AndroidBitmapInfo bmpInfo = AndroidBitmapInfo{0};
    AndroidBitmap_getInfo(env, bmp_out, &bmpInfo);
    AndroidBitmap_lockPixels(env, bmp_out, (void**)&bmp_ptr);
    tmp = new array2d<uint8_t>(rad_N, n_line, true);
    bmp = new array2d<uint32_t>(bmpInfo.width, bmpInfo.height, false);
    bmp->set_ptr(bmp_ptr);
    ls_para = new line_search_para{tmp,                                                   //out
                                   point_u32(start_x, start_y),                     //point
                                   rads_range{rad_start, rad_step, {0,static_cast<uint32_t>(rad_N)}},
                                   int_range{0, static_cast<uint32_t>(n_line)},   //N_line
                                   static_cast<uint32_t>(n_length)};                      //N_length
    init(n_thread, ls_para, in_width, in_height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_frame_1hist2d(JNIEnv *env, jclass clazz,
                jlong mat_addr_old, jlong mat_addr_new, jlong mat_hist) {
    Mat& img_old = *(Mat*)mat_addr_old;
    Mat& img_new = *(Mat*)mat_addr_new;
    Mat& out_hist= *(Mat*)mat_hist;
    int total_pix = img_new.cols * img_new.rows;
    uchar *p_old = img_old.ptr();
    uchar *p_new = img_new.ptr();
    auto *p_hist= out_hist.ptr<ushort>();
    ushort *p_hist_end = p_hist + out_hist.cols*out_hist.rows*out_hist.channels();
    while(p_hist < p_hist_end) {
        *p_hist = 0;
        p_hist++;
    }
    p_hist = out_hist.ptr<ushort>();
    for(int i=0;i<total_pix;i++) {
        for(int j=0;j<3;j++) {
            *(p_hist + (*p_old)*0x100*4 + (*p_new)*4 + j) += 1;
            p_new++;
            p_old++;
        }
        p_new++;
        p_old++;
    }

    // TODO remove the codes under, add a view to show hist2d
    p_new = img_new.ptr();
    int p_new_row  =  img_new.cols * img_new.channels();
    int p_new_col  = img_new.channels();
    int p_hist_row = out_hist.cols *out_hist.channels();
    int p_hist_col = out_hist.channels();
    int bias_new, bias_hist;
    assert(p_hist_row < p_new_row);
    for(int i=0;i<out_hist.rows;i++) {
        /*memcpy(p_new + p_new_row_bias*i,      //the code is u8 to u8, now is u16 to u8
               p_hist + p_hist_row_bias*i,
               sizeof(uchar)*p_hist_row_bias);*/
        for(int j=0;j<out_hist.cols;j++) {
            bias_new  =  p_new_row*i +  p_new_col*j;
            bias_hist = p_hist_row*i + p_hist_col*j;
            *(p_new + bias_new + 0) = (*(p_hist + bias_hist + 0))>>2U;
            *(p_new + bias_new + 1) = (*(p_hist + bias_hist + 1))>>2U;
            *(p_new + bias_new + 2) = (*(p_hist + bias_hist + 2))>>2U;
        }
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_OptFlow_1init(JNIEnv *env, jclass clazz,
            jint rows, jint cols) {
    optflow = new OptFlow(rows, cols);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_OptFlow_1LK(JNIEnv *env, jclass clazz,
        jlong mat_addr, jobject status) {
    long t_ns;
    jfieldID id;
    jclass c = env->FindClass("com/example/sh3dscaner/ImageProcess$Status");
    id = env->GetFieldID(c, "process_time", "J");
    Mat& mat = *(Mat*)mat_addr;
    if(N_frames%30 == 0)
        optflow->getFeat(mat);
    else{
        t_ns = optflow->update(mat);
        env->SetLongField(status, id, t_ns);
    }
    N_frames += 1;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_optflow_1FFT_1init(JNIEnv *env, jclass clazz, jint out_n,
                                                            jint x0, jint y0) {
    OF_fft = new optflow_FFT(64);

    point_int[0].x=x0;
    point_int[0].y=y0;
    point_int[1].x=x0+64;
    point_int[1].y=y0;
    point_int[2].x=x0+64;
    point_int[2].y=y0+64;
    point_int[3].x=x0;
    point_int[3].y=y0+64;

    small.create(16, 16, CV_8UC1);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_optflow_1FFT_1update(JNIEnv *env, jclass clazz,
               jlong mat_addr, jlong out_mat, jobject status) {
    Scalar color = Scalar(0, 50, 200);
    timespec ts0{}, ts1{};
    Mat& mat = *(Mat*)mat_addr;
    Mat gray;
    Mat& omat = *(Mat*)out_mat;
    jfieldID id;
    jclass c = env->FindClass("com/example/sh3dscaner/ImageProcess$Status");
    id = env->GetFieldID(c, "process_time", "J");
    cvtColor(mat, gray, COLOR_RGBA2GRAY);
    OF_fft->fill_data(gray, point_int[0].x, point_int[0].y);
    clock_gettime(CLOCK_REALTIME, &ts0);
    if(AB){
        OF_fft->run(0);
    }
    else{
        OF_fft->run(1);
        OF_fft->calc_delta();
        OF_fft->run(2);
        OF_fft->out_ifft(&small);
        resize(small, omat, Size(),
               omat.cols/small.cols, omat.rows/small.rows,
               INTER_NEAREST);
        //OF_fft->copy_result(omat);
    }
    clock_gettime(CLOCK_REALTIME, &ts1);
    AB =! AB;
    long t_ns = ts1.tv_nsec - ts0.tv_nsec;
    env->SetLongField(status, id, t_ns);
    line(mat, point_int[0], point_int[1], color, 1, LINE_4);
    line(mat, point_int[1], point_int[2], color, 1, LINE_4);
    line(mat, point_int[2], point_int[3], color, 1, LINE_4);
    line(mat, point_int[3], point_int[0], color, 1, LINE_4);
}