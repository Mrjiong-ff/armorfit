#ifndef ARMORFIT_H
#define ARMORFIT_H
#include<opencv2/opencv.hpp>
#include<iostream>
using namespace std;
using namespace cv;
class Nicelight{
public:
    void inputArmor(RotatedRect &_rect){
        this->rect = _rect;
        this->Area = _rect.size.width * _rect.size.height;
    }
    RotatedRect rect = RotatedRect();
    float Area = 0;
    void deal(Mat frame);
    void armorFitted();
};

#endif // ARMORFIT_H
