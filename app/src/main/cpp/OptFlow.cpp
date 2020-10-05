//
// Created by xrj on 20-9-22.
//
//参考 https://www.cnblogs.com/logo-88/p/9456117.html
#include "OptFlow.h"
#include <opencv2/core.hpp>
#include "opencv2/video/tracking.hpp"
#include <vector>
using namespace cv;
using namespace std;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
OptFlow::OptFlow(int rows, int cols) {
    gray_mats[0].create(rows, cols, CV_8UC1);
    gray_mats[1].create(rows, cols, CV_8UC1);
    gray = new circle_queue<Mat>(gray_mats, 2);

    criteria = TermCriteria(TermCriteria::COUNT|TermCriteria::EPS, 20, 0.03);
    color = Scalar(0,50,200);
}
#pragma clang diagnostic pop

void OptFlow::getFeat(Mat &img)
{
    double qualityLevel = 0.01;
    double minDistance = 10;
    int MAX_CORNERS = 500;
    cvtColor(img, gray->next(), COLOR_RGBA2GRAY);
    goodFeaturesToTrack((*gray)[0], point[0], MAX_CORNERS, qualityLevel, minDistance);
}

inline void point_f2i(Point2f &point_float, Point2i &point_int) {
    point_int.x = cvRound(point_float.x);
    point_int.y = cvRound(point_float.y);
}

long OptFlow::update(Mat &img)
{
    timespec ts0{}, ts1{};
    cvtColor(img, gray->next(), COLOR_RGBA2GRAY);

    clock_gettime(CLOCK_REALTIME, &ts0);
    calcOpticalFlowPyrLK((*gray)[-1], (*gray)[0],
            point[gray->i], point[!gray->i],
            status, err,
            Size(11, 11),
            3, criteria);
    clock_gettime(CLOCK_REALTIME, &ts1);

    int m = min(point[0].size(), point[1].size());
    Point2i point_int[2];
    for(int i=0;i<m;i++) {
        point_f2i(point[0][i], point_int[0]);
        point_f2i(point[1][i], point_int[1]);
        line(img, point_int[0], point_int[1], color, 1, LINE_AA);
    }
    return ts1.tv_nsec - ts0.tv_nsec;
}
