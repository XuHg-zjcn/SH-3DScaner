//
// Created by xrj on 20-9-22.
//

#ifndef SH_3DScaner_OPTFLOW_H
#define SH_3DScaner_OPTFLOW_H

#include <opencv2/core.hpp>
#include <vector>
#include <queue>
using namespace cv;
using namespace std;

template<typename T>
class circle_queue {
private:
    T* ptr0;
    int N;
public:
    int i;
    circle_queue(T* ptr, int N) {
        this->ptr0 = ptr;
        this->N = N;
        this->i = 0;
    }
    T& operator[](int j) {
        int ret_n;
        assert(j<N);
        if(j<0)       //负索引
            j=N+j;
        ret_n = i+j;
        if(ret_n>=N)   //不超出范围 始终保持ret_n<N
            ret_n-=N;
        return *(ptr0+ret_n);
    }
    T& next() {
        i++;
        if(i>=N) //始终保持i<N
            i=0;
        return *(ptr0+i);
    }
};

class OptFlow {
private:
    vector<Point2f> point[2];
    vector<uchar> status;
    vector<float> err;
    TermCriteria criteria;
    Mat gray_mats[2];
    circle_queue<Mat> *gray;
    Scalar color;
public:
    OptFlow(int rows, int cols);
    void getFeat(Mat &img);
    long update(Mat &img);
};


#endif //SH_3DScaner_OPTFLOW_H
