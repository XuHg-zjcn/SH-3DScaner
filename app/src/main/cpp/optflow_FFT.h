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
        void copy_zoom(int width, Mat out);
        void xsum(float dx, float dy, fftwf_complex &ret);
        void copy_result(Mat out);

    protected:
        int save();

    private:
        uint32_t n=0;
        fftwf_plan p1;
        fftwf_plan p2;
        fftwf_plan p_ifft;
        float *crop_db;
        fftwf_complex *out1;
        fftwf_complex *out2;
        fftwf_complex *mul;
        float *ifft;
        Mat show_u8;
};


#endif //SH_3DScaner_OPTFLOW_FFT_H
