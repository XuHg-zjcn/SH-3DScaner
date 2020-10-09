//
// Created by xrj on 20-10-5.
//

#include "optflow_FFT.h"
#include "fftw3.h"
optflow_FFT::optflow_FFT(uint32_t n)
{
    //ctor
    this->n = n;
    crop_db =    (double*)fftw_malloc(sizeof(double)*n*n);
    out1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*n*(n/2+1));
    out2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*n*(n/2+1));
    mul  = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*n*(n/2+1));
    ifft =       (double*)fftw_malloc(sizeof(double)*n*n);
    if(fftw_import_wisdom_from_filename("wisdom.fftw")!=0){
        p1 = fftw_plan_dft_r2c_2d(n, n, crop_db, out1, FFTW_WISDOM_ONLY);
        p2 = fftw_plan_dft_r2c_2d(n, n, crop_db, out2, FFTW_WISDOM_ONLY);
        p_ifft = fftw_plan_dft_c2r_2d(n, n, mul, ifft, FFTW_WISDOM_ONLY);
    }else{
        p1 = fftw_plan_dft_r2c_2d(n, n, crop_db, out1, FFTW_PATIENT);
        p2 = fftw_plan_dft_r2c_2d(n, n, crop_db, out2, FFTW_PATIENT);
        p_ifft = fftw_plan_dft_c2r_2d(n, n, mul, ifft, FFTW_PATIENT);
    }
}

optflow_FFT::~optflow_FFT()
{
    //dtor
    fftw_free(crop_db);
    fftw_free(out1);
    fftw_free(out2);
    fftw_destroy_plan(p1);
    fftw_destroy_plan(p2);
}

void optflow_FFT::run(uint32_t n)
{
    if(n==0)
        fftw_execute(p1);
    else
        fftw_execute(p2);
}

int optflow_FFT::save()
{
    FILE *fp;
    fp = fopen("wisdom.fftw", "w");
    if(fp == nullptr){
        return 1;
    }
    fftw_export_wisdom_to_file(fp);
    fclose(fp);
    return 0;
}

void optflow_FFT::fill_data(Mat &mat_in, uint32_t x0, uint32_t y0)
{
    uint8_t *ptr_row;
    double *ptr_db=crop_db;
    for(uint32_t i=y0;i<y0+n;i++) {
        ptr_row = mat_in.ptr(i, x0);
        for(uint32_t j=0;j<n;j++) {
            *ptr_db++=*ptr_row++;
        }
    }
}

void optflow_FFT::calc_delta()
{
    double mul_real, mul_imag, sqrt2;
    for(uint32_t i=0;i<64*33;i++) {
        mul_real = out1[i][0]*out2[i][0] + out1[i][1]*out2[i][1];
        mul_imag =-out1[i][0]*out2[i][1] + out1[i][1]*out2[i][0];
        sqrt2 = sqrt(mul_real*mul_real + mul_imag*mul_imag);
        mul[i][0] = mul_real/sqrt2;
        mul[i][1] = mul_imag/sqrt2;
    }
    fftw_execute(p_ifft);
}

void optflow_FFT::copy_result(uint32_t* p)
{
    double abs_v;
    for(uint32_t i=0;i<64*64;i++) {
        abs_v = ifft[i]/20;
        abs_v += 32;
        abs_v = abs_v<0   ?   0 : abs_v;
        abs_v = abs_v>255 ? 255 : abs_v;
        *p++ = ((uint8_t)abs_v*0x00010101) + 0xff000000;
    }
    /*for(uint32_t i=0;i<64*33;i++) {
        abs_v = mul[i][1];
        abs_v = abs_v*128 + 128;
        abs_v = abs_v<0   ?   0 : abs_v;
        abs_v = abs_v>255 ? 255 : abs_v;
        *p++ = ((uint8_t)abs_v*0x00010101) + 0xff000000;
    }*/
}
