//
// Created by xrj on 20-10-5.
//

#include <opencv2/imgproc.hpp>
#include "optflow_FFT.h"
#include "fftw3.h"

#define _USE_MATH_DEFINES
#include <cmath>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
optflow_FFT::optflow_FFT(uint32_t n)
{
    //ctor
    this->n = n;
    crop_db =    (float*)fftwf_malloc(sizeof(float)*n*n);
    out1 = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*n*(n/2+1));
    out2 = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*n*(n/2+1));
    mul  = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*n*(n/2+1));
    ifft =       (float*)fftwf_malloc(sizeof(float)*n*n);
    if(fftwf_import_wisdom_from_filename("wisdom.fftw")!=0){
        p1 = fftwf_plan_dft_r2c_2d(n, n, crop_db, out1, FFTW_WISDOM_ONLY);
        p2 = fftwf_plan_dft_r2c_2d(n, n, crop_db, out2, FFTW_WISDOM_ONLY);
        p_ifft = fftwf_plan_dft_c2r_2d(n, n, mul, ifft, FFTW_WISDOM_ONLY);
    }else{
        p1 = fftwf_plan_dft_r2c_2d(n, n, crop_db, out1, FFTW_PATIENT);
        p2 = fftwf_plan_dft_r2c_2d(n, n, crop_db, out2, FFTW_PATIENT);
        p_ifft = fftwf_plan_dft_c2r_2d(n, n, mul, ifft, FFTW_PATIENT);
    }
    show_u8.create(16, 16, CV_8UC1);
}
#pragma clang diagnostic pop

optflow_FFT::~optflow_FFT()
{
    //dtor
    fftwf_free(crop_db);
    fftwf_free(out1);
    fftwf_free(out2);
    fftwf_destroy_plan(p1);
    fftwf_destroy_plan(p2);
}

void optflow_FFT::run(uint32_t n)
{
    if(n==0)
        fftwf_execute(p1);
    else
        fftwf_execute(p2);
}

int optflow_FFT::save()
{
    FILE *fp;
    fp = fopen("wisdom.fftw", "w");
    if(fp == nullptr){
        return 1;
    }
    fftwf_export_wisdom_to_file(fp);
    fclose(fp);
    return 0;
}

void optflow_FFT::fill_data(Mat &mat_in, uint32_t x0, uint32_t y0)
{
    uint8_t *ptr_row;
    float *ptr_db=crop_db;
    for(uint32_t i=y0;i<y0+n;i++) {
        ptr_row = mat_in.ptr(i, x0);
        for(uint32_t j=0;j<n;j++) {
            *ptr_db++=*ptr_row++;
        }
    }
}

void optflow_FFT::calc_delta()
{
    float mul_real, mul_imag, sqrt2;
    for(uint32_t i=0;i<64*33;i++) {
        mul_real = out1[i][0]*out2[i][0] + out1[i][1]*out2[i][1];
        mul_imag =-out1[i][0]*out2[i][1] + out1[i][1]*out2[i][0];
        sqrt2 = sqrt(mul_real*mul_real + mul_imag*mul_imag);
        mul[i][0] = mul_real/sqrt2;
        mul[i][1] = mul_imag/sqrt2;
    }
    //fftw_execute(p_ifft);
}

// @para width: show width in orignal image
// @para out: Mat to save zoomed image
// zoom rate=out.size/width
void optflow_FFT::copy_zoom(int width, Mat out)
{
    copy_result(show_u8);
    Mat Mtemp;
    /*Rect area1 = Rect(0,                  0,                  width, width);
    Rect area2 = Rect(show_u8.cols-width, 0,                  width, width);
    Rect area3 = Rect(show_u8.cols-width, show_u8.rows-width, width, width);
    Rect area4 = Rect(0,                  show_u8.rows-width, width, width);*/
    Rect area = Rect(0,0, width,width);
    Mtemp = show_u8(area);
    resize(Mtemp, out, Size(),
           (float)out.cols/show_u8.cols, (float)out.rows/show_u8.rows,
           INTER_NEAREST);
}

void optflow_FFT::xsum(float dx, float dy, fftwf_complex &ret)
{
    float v;//, ret=0;
    ret[0]=0;
    ret[1]=0;
    for(int i=0;i<33;i++) {
        for(int j=0;j<33;j++) {
            v = (float)(i*dx+j*dy)/64*2*M_PI;
            ret[0] += mul[33*i+j][0]*cos(v) - mul[33*i+j][1]*sin(v);
            ret[1] += mul[33*i+j][0]*sin(v) + mul[33*i+j][1]*cos(v);
        }
    }
    ret[0]/=33*64;
    ret[1]/=33*64;
    ret[0]= sqrt(ret[0]*ret[0] + ret[1]*ret[1]);
}

void optflow_FFT::copy_result(Mat out)
{
    uint8_t *p = out.ptr();
    float v,vmin=0,vmax=0;
    auto *vars=(fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*out.cols*out.rows);
    fftwf_complex *vars1=vars;

    for(int i=-out.rows/2;i<out.rows/2;i++) {
        for(int j=-out.cols/2;j<out.cols/2;j++) {
            xsum(i/1.5, j/1.5, *vars1++);
        }
    }
    vars1=vars;
    for(int i=0;i<out.cols*out.rows;i++) {
        v = (*vars1++)[0];
        if(v<vmin)
            vmin=v;
        else if(v>vmax)
            vmax=v;
    }
    vars1=vars;
    for(int i=0;i<out.cols*out.rows;i++) {
        v = (*vars1++)[0];
        v = (v-vmin)/(vmax-vmin)*255;
        v = v<0   ?   0 : v;
        v = v>255 ? 255 : v;
        *p++ = (uint8_t)v;
    }
    fftwf_free(vars);
}
