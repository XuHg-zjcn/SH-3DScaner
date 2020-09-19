//
// Created by xrj on 20-9-8.
//

#ifndef SH_3D_Scaner_IMAGEPROCESS_H
#define SH_3D_Scaner_IMAGEPROCESS_H

#include <stdarg.h>
#include <cstdint>
#include <cstdlib>
#include <semaphore.h>

extern pthread_mutex_t mutex;
template<typename T>
struct xy{
    T x;
    T y;
};
typedef struct{
    uint32_t a;
    uint32_t b;
}int_range;
typedef struct{
    float start;
    float step;
    int_range range;
}rads_range;

/*typedef struct {
    alignas(8)
    uint16_t H16;
    uint8_t BL8;
    uint8_t u8;
}unpack_u32;*/
typedef struct {
    xy<uint32_t> H16;
    xy<uint32_t> BL8;
}H16_BL8;

class point_u32: public xy<uint32_t>{
public:
    point_u32(uint32_t set_x, uint32_t set_y);
    point_u32(point_u32 *old);
    inline void copy_from_old(point_u32 *old);
    inline void add_delta(xy<int> *delta);
    inline void update_H16_BL8(H16_BL8 *upd);
};

template<typename T>
class array2d:public xy<uint32_t> {
public:
    T *data;
    array2d<T>(int row, int col, bool is_malloc) {
        set_xy(row, col);
        if(is_malloc)
            this->data = (T*)malloc(sizeof(T)*row*col);
        else
            this->data = nullptr;
    }
    array2d<T>(int row, int col, T *ptr) {
        set_xy(row, col);
        this->data = ptr;
    }
    inline uint64_t prod() {
        return x*y;
    }
    inline void set_ptr(T *p) {
        this->data = p;
    }
    inline void set_xy(int row, int col) {
        this->x = col;
        this->y = row;
    }
    inline T* get(uint32_t i, uint32_t j) {
        return data + (x*i) + j;
    }
};

typedef struct {
    array2d<uint8_t> *out;   //detection output, each point is original a line
    point_u32 start;        //search start xy
    rads_range rads;        //
    int_range N_line;       //number of line, output Shape[2]
    uint32_t N_length;      //points per line
}line_search_para;

void init(int N_thread, line_search_para *paras, uint32_t in_width, uint32_t in_height);
int update(uint8_t *img_ptr);

#endif
