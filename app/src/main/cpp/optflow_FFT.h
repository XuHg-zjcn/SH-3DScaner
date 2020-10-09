//
// Created by xrj on 20-10-5.
//

#ifndef SH_3DScaner_OPTFLOW_FFT_H
#define SH_3DScaner_OPTFLOW_FFT_H

#include <cstdint>
#include "fftw3.h"
#include "opencv2/core.hpp"

using namespace cv;
class optflow_FFT
{
    public:
        optflow_FFT(uint32_t n);
        virtual ~optflow_FFT();
        void run(uint32_t n);
        void fill_data(Mat &in, uint32_t x0, uint32_t y0);
        void calc_delta();
        void copy_result(uint32_t* p);

    protected:
        int save();

    private:
        uint32_t n=0;
        fftw_plan p1;
        fftw_plan p2;
        fftw_plan p_ifft;
        double *crop_db;
        fftw_complex *out1;
        fftw_complex *out2;
        fftw_complex *mul;
        double *ifft;
};


#endif //SH_3DScaner_OPTFLOW_FFT_H
