#include<armorfit.h>

using namespace cv;
using namespace std;

Mat bin_img_final;
Nicelight light;
void Nicelight::deal(Mat frame){
    Mat gray_img,bin_img_color,bin_img_gray;
    cvtColor(frame,gray_img,COLOR_BGR2GRAY);
    vector<Mat> _split;
    split(frame, _split);
    subtract( _split[0], _split[2],bin_img_color);
    threshold(gray_img,bin_img_gray,80,255, THRESH_BINARY);
    threshold(bin_img_color,bin_img_color,135,255,THRESH_BINARY);
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(3,7));
    dilate(bin_img_gray,bin_img_gray,element);
    medianBlur(bin_img_color,bin_img_color,5);
    dilate(bin_img_color,bin_img_color,element);
    bitwise_and(bin_img_color,bin_img_gray,bin_img_final);
    imshow("dealfinal",bin_img_final);
}
void Nicelight::armorFitted(){
    vector<Nicelight>good_lights;
    vector<vector <Point> >contours;
    vector<Vec4i>hierarchy;
    findContours(bin_img_final,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_NONE);
    for(auto i = 0;i < contours.size();i++){
        if(contours[i].size()>6){
            break;
        }else{
            continue;
        }
        RotatedRect light_rect = fitEllipse(contours[i]);
        float width = MIN(light_rect.size.width , light_rect.size.height);
        float height = MAX(light_rect.size.width , light_rect.size.height);
        float light_rect_area = width * height;
        float wh_ratio = width / height;
        float lenth = arcLength(contours[i],true);
        if((wh_ratio < 0.4f) && ((0 <= light_rect.angle && light_rect.angle <= 45)||
           (135 <= light_rect.angle && light_rect.angle <= 180)) &&
                light_rect_area > 50 && lenth > 10){
            light.inputArmor(light_rect);
            good_lights.push_back(light);
        }
    }
}
int main(){

}
