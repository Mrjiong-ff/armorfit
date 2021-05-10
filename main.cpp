//#include<armorfit.h>

//using namespace cv;
//using namespace std;

//Nicelight light,left_light,right_light;
//void armorfit::deal(Mat frame){
//    cvtColor(frame,gray_img,COLOR_BGR2GRAY);
//    vector<Mat> _split;
//    split(frame, _split);
//    subtract( _split[0], _split[2],bin_img_color);
//    threshold(gray_img,bin_img_gray,80,255, THRESH_BINARY);
//    threshold(bin_img_color,bin_img_color,135,255,THRESH_BINARY);
//    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(3,7));
//    dilate(bin_img_gray,bin_img_gray,element);
//    medianBlur(bin_img_color,bin_img_color,5);
//    dilate(bin_img_color,bin_img_color,element);
//    bitwise_and(bin_img_color,bin_img_gray,bin_img_final);
//    imshow("dealfinal",bin_img_final);
//}
//void armorfit::armorFitted(){
//    vector<Nicelight>good_lights;
//    vector<vector <Point> >contours;
//    vector<Vec4i>hierarchy;
//    findContours(bin_img_final,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_NONE);
//    for(auto i = 0;i < contours.size();i++){
//        if(contours[i].size()>6){
//            break;
//        }
//        RotatedRect light_rect = fitEllipse(contours[i]);
//        float width = MIN(light_rect.size.width , light_rect.size.height);
//        float height = MAX(light_rect.size.width , light_rect.size.height);
//        float light_rect_area = width * height;
//        float wh_ratio = width / height;
//        float lenth = arcLength(contours[i],true);
//        if((wh_ratio < 0.4f) && ((0 <= light_rect.angle && light_rect.angle <= 45)||
//           (135 <= light_rect.angle && light_rect.angle <= 180)) &&
//                light_rect_area > 50 && lenth > 10){
//            light.inputArmor(light_rect);
//            good_lights.push_back(light);
//        }
//    }
//    RotatedRect armor_rect;
//    for(auto i = 0; i < good_lights.size();i++){
//        for(auto j = i+1;j < good_lights.size();j++){
//            if(good_lights[i].rect.center.x <
//                    good_lights[j].rect.center.x){
//                left_light = good_lights[i];
//                right_light = good_lights[j];
//            }else{
//                left_light = good_lights[j];
//                right_light = good_lights[i];
//            }
//            float w1 = MIN(left_light.rect.size.width , left_light.rect.size.height);
//            float w2 = MIN(right_light.rect.size.width , right_light.rect.size.height);
//            float h1 = MAX(left_light.rect.size.width , left_light.rect.size.height);
//            float h2 = MAX(right_light.rect.size.width , right_light.rect.size.height);
//            //宽高
//            float angle_left = left_light.rect.angle;
//            float angle_right = right_light.rect.angle;
//            //角度
//            float light_h_diff = fabs(left_light.rect.center.y - right_light.rect.center.y);
//            bool y_distance_catch = (light_h_diff < (h1 + h2)* 0.5f);
//            //高度差
//            float light_dis = centerDis(left_light.rect.center , right_light.rect.center);
//            bool x_distance_catch = (light_dis < MAX(h1,h2) * 5);
//            //距离差
//            bool light_angle_catch = (light_dis < MAX(h1,h2) * 5);
//            if((angle_left < 90.f && angle_right < 90.f)||
//                    (angle_left > 90.f && angle_right < 90.f)){
//                light_angle_catch = (fabs(angle_left - angle_right) <= 10.f);
//            }else if(angle_left > 90.f && angle_right < 90.f){
//                light_angle_catch = (170.f <= fabs(angle_left - angle_right) &&
//                                     fabs(angle_left - angle_right) <= 180.f);
//            }else if(angle_left < 90.f && angle_right > 90.f){
//                light_angle_catch = (170.f <= fabs(angle_left - angle_right) &&
//                                     fabs(angle_left - angle_right) <= 180.f);
//            }else if(angle_left == 0.f && angle_right != 0.f){
//                light_angle_catch = (170.f <= fabs(angle_left - angle_right) &&
//                                     fabs(angle_left - angle_right) <= 10.f);
//            }else if(angle_left !=0.f && angle_right == 0.f){
//                light_angle_catch = (170.f <= fabs(angle_left - angle_right) &&
//                                     fabs(angle_left - angle_right) <= 10.f);
//            }
//            x_distance_catch = true;
//        }
//    }

//}
//int main(){

//}
#include "Model/RM_Armor/RM_Armor.h"

RM_Armor::RM_Armor(const Armor_Cfg& _armor_config)
    : armor_config(_armor_config), armor(armor_config.candidate_armor_cfg)
{
    this->target_2d.reserve(4);
}

RM_Armor::~RM_Armor() {}

// Light

void Light::inputparam(const RotatedRect& _rect, const size_t& _index)
{
    this->width        = MIN(_rect.size.width, _rect.size.height);
    this->height       = MAX(_rect.size.width, _rect.size.height);
    this->area         = _rect.size.area();
    this->aspect_ratio = _rect.size.height / _rect.size.width;

    if (_rect.angle >= 0.f && _rect.angle <= 90.f) {
        this->angle = _rect.angle + 90.f;
    }
    else if (_rect.angle > 90.f && _rect.angle <= 180.f) {
        this->angle = _rect.angle - 90.f;
    }

    this->center = _rect.center;
    this->index  = _index;
    this->rect   = _rect;
}

void Light::updateIndex(const size_t& _index)
{
    this->index = _index;
}

CandidateArmor::CandidateArmor(const Candidate_Armor_Cfg& _candidate_armor_config)
    : candidate_armor_config(_candidate_armor_config)
{
    this->armor_type_th = this->candidate_armor_config.ARMOR_TYPE_TH;
}

// CandidateArmor
void CandidateArmor::inputparam(const Light&  _left_light,
                                const Light&  _right_light,
                                const size_t& _index,
                                const Point&  _img_center)
{
    // 拟合初步装甲板，获取信息
    this->rect = this->draftTentativeArmor(_left_light, _right_light);

    // 装甲板宽度 可能需要修正
    // 装甲板真实中点
    this->convert_center = rect.center;
    // this->convert_center = rect.center+Point2f(roi.tl);//TODO

    // 装甲板中点距离图像中点信息
    this->dist_to_center = centerDistance(this->convert_center, _img_center);
    // 得到装甲板的旋转矩阵
    this->rect = RotatedRect(convert_center, rect.size, rect.angle);

    // 更新数据
    this->updateCandidateArmor(_left_light, _right_light, _index);
    // 计算深度：略
}

/**
 * @brief 拟合初步装甲板
 * @param  _left_light      左灯条
 * @param  _right_light     右灯条
 * @return RotatedRect      拟合装甲板
 */
RotatedRect CandidateArmor::draftTentativeArmor(const Light& _left_light, const Light& _right_light)
{
    Point2f center   = (_left_light.center + _right_light.center) * 0.5;
    float   distance = centerDistance(_left_light.center, _right_light.center);
    float   w        = distance + (_left_light.width * 0.5 + _right_light.width * 0.5);
    float   h        = MAX(_left_light.height, _right_light.height);

    float       angle = atan2((_left_light.center.y - _right_light.center.y),
                        (_left_light.center.x - _right_light.center.x));
    RotatedRect Rrect = RotatedRect(center, Size2f(w, h), angle * 180 / static_cast<float>(CV_PI));
    return Rrect;
}

/**
 * @brief 更新候选装甲板信息
 * @param  _left_light      左灯条
 * @param  _right_light     右灯条
 * @param  _index           装甲板索引
 */
void CandidateArmor::updateCandidateArmor(const Light&  _left_light,
                                          const Light&  _right_light,
                                          const size_t& _index)
{
    // 更新左右灯条
    this->left_light  = _left_light;
    this->right_light = _right_light;

    // 更新宽度、高度
    this->width  = this->rect.size.width;
    this->height = this->rect.size.height;
    // 更新角度差
    this->diff_angle = fabs(_left_light.angle - _right_light.angle);

    // 更新角度
    if (this->rect.angle < 0.f) {
        this->angle = 360.f + this->rect.angle;
    }
    else {
        this->angle = this->rect.angle;
    }

    // 计算宽高比
    this->aspect_ratio = this->width / this->height;

    // 判断大小装甲板 需要测试
    this->type = this->judgeArmorSize();

    // 更新当前装甲板序号
    this->index = _index;
}

/**
 * @brief 判断大小装甲板
 * @return armor_size
 * @note SMALL_ARMOR  小
 * @note BIG_ARMOR 大
 */
armor_size CandidateArmor::judgeArmorSize()
{
    if (this->aspect_ratio < 3.6) {
        return SMALL_ARMOR;
    }
    else {
        return BIG_ARMOR;
    }
}

/**
 * @brief 总执行函数
 * @param  _frame           处理图像
 * @param  _receive_info    信息传输
 * @return true             有目标
 * @return false            无目标
 */
bool RM_Armor::identificationTask(Mat                 _frame,
                                  const Receive_Info& _receive_info,
                                  RM_Messenger*       _messenger)
{
    // 预处理
    this->imageProcessing(_frame, _receive_info.my_color);

    // 筛选灯条，少于等于一个退出 TODO:更新串口发送信息 坐标，标志位
    if (!this->lightScreenting()) {
        // 根据丢包次数来分发送内容，小于5次还是用回之前剩下的，大于5次才清空
        if (lose_target_cnt < 2) {
            this->final_armor = this->last_armor;
        }
        else {
            _messenger->setSendLostInfo();
        }
        return false;
    }

    // 筛选候选装甲板，少于一个退出 TODO:更新串口发送信息 坐标，标志位
    if (!this->armorScreenting()) {
        if (lose_target_cnt < 2) {
            this->final_armor = this->last_armor;
        }
        else {
            _messenger->setSendLostInfo();
        }
        return false;
    }
    _messenger->getSendInfo().is_found_target =1;
    // 选取最优装甲板,只有一个时可忽略 TODO:更新串口发送信息 坐标，标志位
    this->armorSelectOptimum();

    return true;
}

/**
 * @brief  预处理
 * @param  _frame           处理图像
 * @param  _my_color        颜色信息
 */
void RM_Armor::imageProcessing(Mat _frame, const int& _my_color)
{
    // 更新图像
    this->src_img = _frame;

    // roi的截取 TODO
    // if：roi的图
    // else：原图

    this->roi_img = src_img;
    this->src_img.copyTo(this->dst_img);

    // 调参时需要显示的灯条图像
    if (this->armor_config.DRAW_LIGHT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        this->src_img.copyTo(this->light_img);
    }
    // 调参时需要显示的装甲板图像
    if (this->armor_config.DRAW_ARMOR_RECT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        this->src_img.copyTo(this->armor_img);
    }

    cvtColor(this->roi_img, this->gray_img, COLOR_BGR2GRAY);

    // 选择预处理的模式：BGR、HSV、LAB
    switch (this->armor_config.PROCESSING_MODE) {
        case BGR_MODE: {
            this->BGRProcessing(_my_color);
            break;
        }
        case HSV_MODE: {
            this->HSVProcessing(_my_color);
            break;
        }
        case LAB_MODE: {
            this->LABProcessing(_my_color);
            break;
        }
        default: {
            this->HSVProcessing(_my_color);
            break;
        }
    }

    // 显示各部分二值图
    if (this->armor_config.SHOW_BIN_IMG == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("bin_img_color", bin_img_color);
        imshow("bin_img_gray", bin_img_gray);
    }

    // 求交集
    bitwise_and(bin_img_color, bin_img_gray, bin_img_color);

    // 显示最终合成二值图
    if (this->armor_config.SHOW_BIN_IMG == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("bin_img_final", bin_img_color);
    }
}

/**
 * @brief BGR color space
 * @param  _my_color        own color
 */
void RM_Armor::BGRProcessing(const int& _my_color)
{
    split(this->roi_img, this->split_img);

    switch (_my_color) {
        case RED: {
            /* my_color为红色，则处理蓝色的情况 */
            /* 灰度图与RGB同样做蓝色处理 */
            subtract(split_img[0], split_img[2], bin_img_color);  // b-r
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_BLUE:", "trackbar",
                               &this->armor_config.BLUE_ARMOR_COLOR_TH, 255, nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 BGR蓝色预处理调参面板已打开 🧐" << endl;
            }
            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.BLUE_ARMOR_GRAY_TH,
                      255, THRESH_BINARY);
            // 颜色部分

            threshold(this->bin_img_color, this->bin_img_color,
                      this->armor_config.BLUE_ARMOR_COLOR_TH, 255, THRESH_BINARY);
            break;
        }
        case BLUE: {
            /* my_color为蓝色，则处理红色的情况 */
            /* 灰度图与RGB同样做红色处理 */
            subtract(split_img[2], split_img[0], bin_img_color);  // r-b
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_COLOR_TH,
                               255, nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 BGR红色预处理调参面板已打开 🧐" << endl;
            }
            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.RED_ARMOR_GRAY_TH, 255,
                      THRESH_BINARY);
            // 颜色部分
            threshold(this->bin_img_color, this->bin_img_color,
                      this->armor_config.RED_ARMOR_COLOR_TH, 255, THRESH_BINARY);

            break;
        }
        default: {
            subtract(this->split_img[0], this->split_img[2], bin_img_color1);  // b-r
            subtract(this->split_img[2], this->split_img[0], bin_img_color2);  // r-b
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_COLOR_TH,
                               255, nullptr);
                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_BLUE:", "trackbar",
                               &this->armor_config.BLUE_ARMOR_COLOR_TH, 255, nullptr);
                cout << "🧐 BGR通用预处理调参面板已打开 🧐" << endl;
            }

            // 亮度部分
            this->average_th = static_cast<int>(
                (this->armor_config.RED_ARMOR_GRAY_TH + this->armor_config.BLUE_ARMOR_GRAY_TH) *
                0.5);
            threshold(this->gray_img, this->bin_img_gray, average_th, 255, THRESH_BINARY);
            // 颜色部分
            threshold(this->bin_img_color1, this->bin_img_color1,
                      this->armor_config.BLUE_ARMOR_COLOR_TH, 255, THRESH_BINARY);
            threshold(this->bin_img_color2, this->bin_img_color2,
                      this->armor_config.RED_ARMOR_COLOR_TH, 255, THRESH_BINARY);
            // 求并集
            bitwise_or(bin_img_color1, bin_img_color2, bin_img_color);
            break;
        }
    }

    split_img.clear();
    vector<Mat>(split_img).swap(split_img);  // TODO:查看容量有多大
}

/**
 * @brief HSV color space
 * @param  _my_color        own color
 */
void RM_Armor::HSVProcessing(const int& _my_color)
{
    cvtColor(this->roi_img, this->hsv_img, COLOR_BGR2HSV_FULL);

    switch (_my_color) {
        case RED:
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_BLUE_MAX:", "trackbar", &this->armor_config.H_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("H_BLUE_MIN:", "trackbar", &this->armor_config.H_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("S_BLUE_MAX:", "trackbar", &this->armor_config.S_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("S_BLUE_MIN:", "trackbar", &this->armor_config.S_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("V_BLUE_MAX:", "trackbar", &this->armor_config.V_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("V_BLUE_MIN:", "trackbar", &this->armor_config.V_BLUE_MIN, 255,
                               nullptr);
                cout << "🧐 HSV蓝色预处理调参面板已打开 🧐" << endl;
            }
            // 颜色部分
            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_BLUE_MIN, this->armor_config.S_BLUE_MIN,
                           this->armor_config.V_BLUE_MIN),
                    Scalar(this->armor_config.H_BLUE_MAX, this->armor_config.S_BLUE_MAX,
                           this->armor_config.V_BLUE_MAX),
                    bin_img_color);
            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.BLUE_ARMOR_GRAY_TH,
                      255, THRESH_BINARY);

            break;
        case BLUE:
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_RED_MAX:", "trackbar", &this->armor_config.H_RED_MAX, 360,
                               nullptr);
                createTrackbar("H_RED_MIN:", "trackbar", &this->armor_config.H_RED_MIN, 360,
                               nullptr);
                createTrackbar("S_RED_MAX:", "trackbar", &this->armor_config.S_RED_MAX, 255,
                               nullptr);
                createTrackbar("S_RED_MIN:", "trackbar", &this->armor_config.S_RED_MIN, 255,
                               nullptr);
                createTrackbar("V_RED_MAX:", "trackbar", &this->armor_config.V_RED_MAX, 255,
                               nullptr);
                createTrackbar("V_RED_MIN:", "trackbar", &this->armor_config.V_RED_MIN, 255,
                               nullptr);
                cout << "🧐 HSV红色预处理调参面板已打开 🧐" << endl;
            }
            // 颜色部分
            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_RED_MIN, this->armor_config.S_RED_MIN,
                           this->armor_config.V_RED_MIN),
                    Scalar(this->armor_config.H_RED_MAX, this->armor_config.S_RED_MAX,
                           this->armor_config.V_RED_MAX),
                    bin_img_color);
            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.RED_ARMOR_GRAY_TH, 255,
                      THRESH_BINARY);

            break;
        default:
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");

                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_RED_MAX:", "trackbar", &this->armor_config.H_RED_MAX, 360,
                               nullptr);
                createTrackbar("H_RED_MIN:", "trackbar", &this->armor_config.H_RED_MIN, 360,
                               nullptr);
                createTrackbar("S_RED_MAX:", "trackbar", &this->armor_config.S_RED_MAX, 255,
                               nullptr);
                createTrackbar("S_RED_MIN:", "trackbar", &this->armor_config.S_RED_MIN, 255,
                               nullptr);
                createTrackbar("V_RED_MAX:", "trackbar", &this->armor_config.V_RED_MAX, 255,
                               nullptr);
                createTrackbar("V_RED_MIN:", "trackbar", &this->armor_config.V_RED_MIN, 255,
                               nullptr);

                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_BLUE_MAX:", "trackbar", &this->armor_config.H_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("H_BLUE_MIN:", "trackbar", &this->armor_config.H_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("S_BLUE_MAX:", "trackbar", &this->armor_config.S_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("S_BLUE_MIN:", "trackbar", &this->armor_config.S_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("V_BLUE_MAX:", "trackbar", &this->armor_config.V_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("V_BLUE_MIN:", "trackbar", &this->armor_config.V_BLUE_MIN, 255,
                               nullptr);
                cout << "🧐 HSV通用预处理调参面板已打开 🧐" << endl;
            }

            // 亮度部分
            this->average_th = static_cast<int>(
                (this->armor_config.RED_ARMOR_GRAY_TH + this->armor_config.BLUE_ARMOR_GRAY_TH) *
                0.5);
            threshold(this->gray_img, this->bin_img_gray, average_th, 255, THRESH_BINARY);

            // 红色
            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_RED_MIN, this->armor_config.S_RED_MIN,
                           this->armor_config.V_RED_MIN),
                    Scalar(this->armor_config.H_RED_MAX, this->armor_config.S_RED_MAX,
                           this->armor_config.V_RED_MAX),
                    bin_img_color2);
            // 蓝色
            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_BLUE_MIN, this->armor_config.S_BLUE_MIN,
                           this->armor_config.V_BLUE_MIN),
                    Scalar(this->armor_config.H_BLUE_MAX, this->armor_config.S_BLUE_MAX,
                           this->armor_config.V_BLUE_MAX),
                    bin_img_color1);

            // 求并集
            bitwise_or(bin_img_color1, bin_img_color2, bin_img_color);
            break;
    }
}

/**
 * @brief LAB color space
 * @param  _my_color        own color
 */
void RM_Armor::LABProcessing(const int& _my_color)
{
    cvtColor(this->roi_img, this->lab_img, COLOR_BGR2Lab);

    switch (_my_color) {
        case RED:
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("L_RED_MAX:", "trackbar", &this->armor_config.L_RED_MAX, 100,
                               nullptr);
                createTrackbar("L_RED_MIN:", "trackbar", &this->armor_config.L_RED_MIN, 100,
                               nullptr);
                createTrackbar("A_RED_MAX:", "trackbar", &this->armor_config.A_RED_MAX, 255,
                               nullptr);
                createTrackbar("A_RED_MIN:", "trackbar", &this->armor_config.A_RED_MIN, 255,
                               nullptr);
                createTrackbar("B_RED_MAX:", "trackbar", &this->armor_config.B_RED_MAX, 255,
                               nullptr);
                createTrackbar("B_RED_MIN:", "trackbar", &this->armor_config.B_RED_MIN, 255,
                               nullptr);
            }
            // 颜色部分
            inRange(this->lab_img,
                    Scalar(this->armor_config.L_RED_MIN, this->armor_config.A_RED_MIN,
                           this->armor_config.B_RED_MIN),
                    Scalar(this->armor_config.L_RED_MAX, this->armor_config.A_RED_MAX,
                           this->armor_config.B_RED_MAX),
                    bin_img_color);
            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.RED_ARMOR_GRAY_TH, 255,
                      THRESH_BINARY);

            break;
        case BLUE:
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("L_BLUE_MAX:", "trackbar", &this->armor_config.L_BLUE_MAX, 100,
                               nullptr);
                createTrackbar("L_BLUE_MIN:", "trackbar", &this->armor_config.L_BLUE_MIN, 100,
                               nullptr);
                createTrackbar("A_BLUE_MAX:", "trackbar", &this->armor_config.A_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("A_BLUE_MIN:", "trackbar", &this->armor_config.A_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("B_BLUE_MAX:", "trackbar", &this->armor_config.B_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("B_BLUE_MIN:", "trackbar", &this->armor_config.B_BLUE_MIN, 255,
                               nullptr);
            }
            // 颜色部分
            inRange(this->lab_img,
                    Scalar(this->armor_config.L_BLUE_MIN, this->armor_config.A_BLUE_MIN,
                           this->armor_config.B_BLUE_MIN),
                    Scalar(this->armor_config.L_BLUE_MAX, this->armor_config.A_BLUE_MAX,
                           this->armor_config.B_BLUE_MAX),
                    bin_img_color);
            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.BLUE_ARMOR_GRAY_TH,
                      255, THRESH_BINARY);

            break;
        default:
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");

                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("L_RED_MAX:", "trackbar", &this->armor_config.L_RED_MAX, 100,
                               nullptr);
                createTrackbar("L_RED_MIN:", "trackbar", &this->armor_config.L_RED_MIN, 100,
                               nullptr);
                createTrackbar("A_RED_MAX:", "trackbar", &this->armor_config.A_RED_MAX, 255,
                               nullptr);
                createTrackbar("A_RED_MIN:", "trackbar", &this->armor_config.A_RED_MIN, 255,
                               nullptr);
                createTrackbar("B_RED_MAX:", "trackbar", &this->armor_config.B_RED_MAX, 255,
                               nullptr);
                createTrackbar("B_RED_MIN:", "trackbar", &this->armor_config.B_RED_MIN, 255,
                               nullptr);

                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("L_BLUE_MAX:", "trackbar", &this->armor_config.L_BLUE_MAX, 100,
                               nullptr);
                createTrackbar("L_BLUE_MIN:", "trackbar", &this->armor_config.L_BLUE_MIN, 100,
                               nullptr);
                createTrackbar("A_BLUE_MAX:", "trackbar", &this->armor_config.A_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("A_BLUE_MIN:", "trackbar", &this->armor_config.A_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("B_BLUE_MAX:", "trackbar", &this->armor_config.B_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("B_BLUE_MIN:", "trackbar", &this->armor_config.B_BLUE_MIN, 255,
                               nullptr);
            }

            // 亮度部分
            this->average_th = static_cast<int>(
                (this->armor_config.RED_ARMOR_GRAY_TH + this->armor_config.BLUE_ARMOR_GRAY_TH) *
                0.5);
            threshold(this->gray_img, this->bin_img_gray, average_th, 255, THRESH_BINARY);

            // 红色
            inRange(this->lab_img,
                    Scalar(this->armor_config.L_RED_MIN, this->armor_config.A_RED_MIN,
                           this->armor_config.B_RED_MIN),
                    Scalar(this->armor_config.L_RED_MAX, this->armor_config.A_RED_MAX,
                           this->armor_config.B_RED_MAX),
                    bin_img_color2);
            // 蓝色
            inRange(this->lab_img,
                    Scalar(this->armor_config.L_BLUE_MIN, this->armor_config.A_BLUE_MIN,
                           this->armor_config.B_BLUE_MIN),
                    Scalar(this->armor_config.L_BLUE_MAX, this->armor_config.A_BLUE_MAX,
                           this->armor_config.B_BLUE_MAX),
                    bin_img_color1);

            // 求并集
            bitwise_or(bin_img_color1, bin_img_color2, bin_img_color);
            break;
    }
}

/**
 * @brief 筛选灯条
 * @return true             搜索成功
 * @return false            搜索失败
 */
bool RM_Armor::lightScreenting()
{
    findContours(this->bin_img_color, this->contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    // 灯条数量过少直接退出
    if (this->contours.size() < 2) {
        //清空内存
        this->contours.clear();
        vector<vector<Point>>(contours).swap(contours);

        this->is_last_data_catch = false;
        this->is_Lost_target     = true;
        this->lose_target_cnt += 1;
        return false;
    }

    // 调整阈值
    if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        namedWindow("light_trackbar");

        createTrackbar("RATIO_H_W_MAX * 0.01", "light_trackbar", &h_w_ratio_max_int, 1300, nullptr);
        createTrackbar("RATIO_H_W_MIN * 0.01", "light_trackbar", &h_w_ratio_min_int, 1300, nullptr);
        createTrackbar("area", "light_trackbar", &this->armor_config.light_cfg.LIGHT_AREA, 10000,
                       nullptr);
        this->armor_config.light_cfg.RATIO_H_W_MAX = static_cast<float>(h_w_ratio_max_int) * 0.01;
        this->armor_config.light_cfg.RATIO_H_W_MIN = static_cast<float>(h_w_ratio_min_int) * 0.01;
        cout << "🧐 灯条 框选阈值选择面板已打开 🧐" << endl;
    }

    // 遍历灯条
    for (size_t i = 0; i < contours.size(); ++i) {
        // 排查size小于6的
        if (this->contours[i].size() < 6) {
            continue;
        }
        // //计算当前轮廓和下一条轮廓的长度
        // float  lengthcontour = arcLength(i,true);
        // float  lengthcontourn = arcLength((i+1),true);
        // 内接椭圆
        this->rect_tmp = fitEllipse(contours[i]);

        // 准备各种条件所需变量：宽、高、面积、角度
        this->light.inputparam(this->rect_tmp, i);

        // 画出全部可选灯条
        // Point2f vtx[4];
        // this->rect_tmp.points(vtx);
        // for (int j = 0; j < 4; ++j) {
        //     line(src_img, vtx[j], vtx[(j + 1) % 4], Scalar(0, 255, 255), 2, 8, 0);
        // }

        // 条件判断：角度、宽高比、面积大于某个阈值
        if ((this->light.aspect_ratio > this->armor_config.light_cfg.RATIO_H_W_MIN &&
             this->light.aspect_ratio < this->armor_config.light_cfg.RATIO_H_W_MAX) &&
            (this->light.angle >= 55 && this->light.angle <= 125) &&
            (this->/* contours[i].size() */ light.area > this->armor_config.light_cfg.LIGHT_AREA)) {
            // 正确：更新数据、存入容器、画出灯条，显示数据

            this->light_box.emplace_back(light);

            // 画出正确灯条，并显示数据
            // 调参时需要显示的灯条图像
            if (this->armor_config.DRAW_LIGHT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                // TODO:添加roi的偏移

                Point put_angle = Point(this->rect_tmp.center.x, this->rect_tmp.center.y - 40);
                putText(light_img, to_string(this->light.angle), put_angle, FONT_HERSHEY_PLAIN, 1,
                        Scalar(255, 245, 0), 1, 8, false);
                Point put_index = Point(this->rect_tmp.center.x, this->rect_tmp.center.y - 30);
                putText(light_img, to_string(this->light.index), put_index, FONT_HERSHEY_PLAIN, 1,
                        Scalar(0, 255, 0), 1, 8, false);
                Point put_hw_ratio = Point(this->rect_tmp.center.x, this->rect_tmp.center.y - 20);
                putText(light_img, to_string(this->light.aspect_ratio), put_hw_ratio,
                        FONT_HERSHEY_PLAIN, 1, Scalar(180, 105, 255), 1, 8, false);
                Point put_area = Point(this->rect_tmp.center.x, this->rect_tmp.center.y - 10);
                putText(light_img, to_string(this->light.area), put_area, FONT_HERSHEY_PLAIN, 1,
                        Scalar(127, 255, 0), 1, 8, false);

                Point2f vtx[4];
                this->rect_tmp.points(vtx);
                for (int j = 0; j < 4; ++j) {
                    line(light_img, vtx[j], vtx[(j + 1) % 4], Scalar(0, 255, 255), 2, 8, 0);
                }
                cout << "🧐 灯条 框选已打开 🧐" << endl;
            }
        }
        // 错误：遍历下一个
    }

    //清空内存
    this->contours.clear();
    vector<vector<Point>>(contours).swap(contours);

    // 调参时需要显示的灯条图像
    if (this->armor_config.DRAW_LIGHT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("light_img", light_img);
        cout << "🧐 灯条 框选图像已打开 🧐" << endl;
    }

    if (light_box.size() < 2) {
        this->is_last_data_catch = false;
        this->is_Lost_target     = true;
        this->lose_target_cnt += 1;
        return false;
    }

    return true;
}

/**
 * @brief 筛选候选装甲板
 * @return true             搜索成功
 * @return false            搜索失败
 */
bool RM_Armor::armorScreenting()
{
    // 更新标志位 TODO
    this->is_last_data_catch = false;

    if (this->armor_config.IS_PARAM_ADJUSTMENT == 1 && this->armor_config.DRAW_ARMOR_RECT == 1) {
        Point put_size = Point(60, 10);
        putText(this->armor_img, to_string(this->light_box.size()), put_size, FONT_HERSHEY_PLAIN, 1,
                Scalar(0, 255, 155), 1, 8, false);
    }

    // 排序
    sort(this->light_box.begin(), this->light_box.end(),
         [](const Light& l1, const Light& l2) { return l1.center.x < l2.center.x; });

    // TODO:加条件控制显示
    int i = 0;
    for (auto iter = light_box.begin(); iter != light_box.end(); ++iter) {
        (*iter).updateIndex(i);
        i += 1;
        // cout << "center = " << (*iter).center << endl;
        // 显示排序结果
        if (this->armor_config.DRAW_LIGHT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
            Point put_index = Point((*iter).center.x, (*iter).center.y - 30);
            putText(this->armor_img, to_string((*iter).index), put_index, FONT_HERSHEY_PLAIN, 1,
                    Scalar(0, 255, 0), 1, 8, false);
            cout << "🧐 灯条 排序后编号已打开 🧐" << endl;
        }
    }

    // 双指针循环匹配
    this->ra          = this->light_box.size() - 1;
    this->la          = ra - 1;
    this->armor_index = 0;  // index
    this->over_cnt    = 0;

    if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        namedWindow("armor_trackbar");
        // 滑动条修改
        createTrackbar("DIFF_LIGHT_Y * 0.1 = ", "armor_trackbar", &this->diff_light_y, 100,
                       nullptr);
        createTrackbar("DISTANCE_MIN_TH = ", "armor_trackbar",
                       &this->armor_config.candidate_armor_cfg.DISTANCE_MIN_TH, 100, nullptr);
        createTrackbar("DISTANCE_MULTIPLE_TH * 0.1 = ", "armor_trackbar",
                       &this->distance_multiple_th, 80, nullptr);

        createTrackbar("DIFF_ANGLE_TH_SAME * 0.1 = ", "armor_trackbar", &this->diff_angle_th_same,
                       300, nullptr);
        createTrackbar("DIFF_ANGLE_TH_WB * 0.1 = ", "armor_trackbar", &this->diff_angle_th_wb, 300,
                       nullptr);
        createTrackbar("DIFF_ANGLE_TH_NB * 0.1 = ", "armor_trackbar", &this->diff_angle_th_nb, 300,
                       nullptr);
        createTrackbar("DIFF_ANGLE_TH_ZS * 0.1 = ", "armor_trackbar", &this->diff_angle_th_zs, 300,
                       nullptr);
        createTrackbar("DIFF_ANGLE_TH_YS * 0.1 = ", "armor_trackbar", &this->diff_angle_th_ys, 300,
                       nullptr);

        createTrackbar("LIGHT_SIZE_RATIO_MAX * 0.1 = ", "armor_trackbar",
                       &this->light_size_ratio_max, 100, nullptr);
        createTrackbar("LIGHT_SIZE_RATIO_MIN * 0.1 = ", "armor_trackbar",
                       &this->light_size_ratio_min, 100, nullptr);

        createTrackbar("MATCHING_TH  = ", "armor_trackbar",
                       &this->armor_config.candidate_armor_cfg.MATCHING_TH, 10, nullptr);

        // 更新阈值
        this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y = this->diff_light_y;
        this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y *= 0.1;
        this->armor_config.candidate_armor_cfg.DISTANCE_MULTIPLE_TH = this->distance_multiple_th;
        this->armor_config.candidate_armor_cfg.DISTANCE_MULTIPLE_TH *= 0.1;

        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_SAME = this->diff_angle_th_same;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_SAME *= 0.1;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_WB = this->diff_angle_th_wb;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_WB *= 0.1;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_NB = this->diff_angle_th_nb;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_NB *= 0.1;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_ZS = this->diff_angle_th_zs;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_ZS *= 0.1;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_YS = this->diff_angle_th_ys;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_YS *= 0.1;

        this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MIN = this->light_size_ratio_min;
        this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MIN *= 0.1;
        this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MAX = this->light_size_ratio_max;
        this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MAX *= 0.1;

        Point put_diff_light_y = Point(10, 20);
        putText(armor_img, to_string(this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y),
                put_diff_light_y, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
        Point put_distance_multiple_th = Point(10, 30);
        putText(armor_img, to_string(this->armor_config.candidate_armor_cfg.DISTANCE_MULTIPLE_TH),
                put_distance_multiple_th, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
        Point put_armor_type_th = Point(10, 50);
        putText(armor_img, to_string(this->armor_config.candidate_armor_cfg.ARMOR_TYPE_TH),
                put_armor_type_th, FONT_HERSHEY_PLAIN, 1, Scalar(155, 255, 0), 1, 8, false);

        cout << "🧐 装甲板 阈值调整面板已打开 🧐" << endl;
    }

    this->is_include_armor = false;
    while (ra > 0) {
        // 准备匹配的条件计算
        this->light_right = this->light_box[ra];
        this->light_left  = this->light_box[la];
        cout << "ra = " << ra << " la = " << la << endl;

        // 条件一：灯条高度差
        this->light_y_diff = fabs(this->light_left.center.y - this->light_right.center.y);
        this->is_height_diff_catch =
            (this->light_y_diff < (this->light_left.height + this->light_right.height) *
                                      this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y);

        // 条件二：灯条的间距 貌似没什么用？
        this->light_distance   = centerDistance(this->light_left.center, this->light_right.center);
        this->light_height_max = MAX(this->light_left.height, this->light_right.height);
        this->light_height_min = MIN(this->light_left.height, this->light_right.height);
        this->is_light_distance_catch =
            (this->light_distance < this->light_height_max * distance_multiple_th &&
             this->light_distance > this->armor_config.candidate_armor_cfg.DISTANCE_MIN_TH);

        // 条件三：灯条高度比例
        this->is_height_ratio = (this->light_height_max / this->light_height_min) < 1.8;

        // 条件四：灯条大小比例
            #ifdef DEBUG
        this->light_size_ratio =
            MAX(this->light_left.area, this->light_right.area)/MIN(this->light_left.area, this->light_right.area);
        this->is_similar_size =
            this->light_size_ratio < this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MAX &&
            this->light_size_ratio > this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MIN;
            #else
        this->light_size_ratio =
            // light_distance / (MIN(this->light_left.area, this->light_right.area));
            (this->light_right.center.x - (this->light_right.width / 2) - this->light_left.center.x - (this->light_left.width  / 2)) / light_distance;
        this->is_similar_size =
            this->light_size_ratio < this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MAX &&
            this->light_size_ratio > this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MIN;
            #endif // DEBUG

        // 条件五：灯条的角度差：可调
        // 灯条的内外八:兵种按照相应的种类进行选取即可，这里全部列出
        this->is_light_angle_catch = false;
        if ((this->light_left.angle < 90.f && this->light_right.angle < 90.f) ||
            (this->light_left.angle > 90.f && this->light_right.angle > 90.f)) {
            // 同侧
            is_light_angle_catch = (fabs(this->light_left.angle - this->light_right.angle) <=
                                    this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_SAME);
        }
        else if (this->light_left.angle > 90.f && this->light_right.angle < 90.f) {
            // 外八 -- / \ --
            is_light_angle_catch = (fabs(this->light_left.angle - this->light_right.angle) <=
                                    this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_WB);
        }
        else if (this->light_left.angle < 90.f && this->light_right.angle > 90.f) {
            // 内八  -- \ / --
            is_light_angle_catch = (fabs(this->light_left.angle - this->light_right.angle) <=
                                    this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_NB);
        }
        else if (this->light_left.angle == 90.f && this->light_right.angle != 90.f) {
            // 左边竖直
            is_light_angle_catch = (fabs(this->light_left.angle - this->light_right.angle) <=
                                    this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_ZS);
        }
        else if (this->light_left.angle != 90.f && this->light_right.angle == 90.f) {
            // 右边竖直
            is_light_angle_catch = (fabs(this->light_left.angle - this->light_right.angle) <=
                                    this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_YS);
        }
        //条件六：灯条的周长相差量
        this->is_length_catch = false;
        this->light_left.light_arclength = (light_left.width * 2) + (light_left.height * 2) ;
        this->light_right.light_arclength = (light_right .width * 2) + (light_right.height * 2);
        this->light_arclength_diff = fabs(this->light_left.light_arclength - this->light_right.light_arclength);
        if(light_arclength_diff <= 10){
            is_length_catch = true;
        }
        // Debug 绝对不能动
        if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
            namedWindow("armor_trackbar");
            createTrackbar("is_enable_light_y_diff", "armor_trackbar", &is_enable_diff_light_y, 1,
                           nullptr);
            createTrackbar("is_enable_distance_multiple_th", "armor_trackbar",
                           &is_enable_distance_multiple_th, 1, nullptr);
            createTrackbar("is_enable_diff_angle_th", "armor_trackbar", &is_enable_diff_angle_th, 1,
                           nullptr);
            createTrackbar("is_enable_similar_size", "armor_trackbar", &is_enable_similar_size, 1,
                           nullptr);
            createTrackbar("is_height_ratio", "armor_trackbar", &is_enable_height_ratio, 1,
                           nullptr);

            if (is_enable_diff_angle_th == 1) {
                this->is_light_angle_catch = true;
            }
            if (is_enable_diff_light_y == 1) {
                this->is_height_diff_catch = true;
            }
            if (is_enable_distance_multiple_th == 1) {
                this->is_light_distance_catch = true;
            }
            if (is_enable_similar_size == 1) {
                this->is_similar_size = true;
            }
            if (is_enable_height_ratio == 1) {
                this->is_height_ratio = true;
            }
            cout << "🧐 装甲板 条件选择使能面板已打开 🧐" << endl;
        }

        // 匹配条件的筛选 一个个来
        if (is_height_diff_catch && is_light_distance_catch && is_light_angle_catch &&
            is_similar_size && is_height_ratio && is_length_catch) {
            // 匹配成功则滑动窗口
            ra = la - 1;

            if (this->is_include_armor) {
                ra               = include_armor_index;
                is_include_armor = false;
            }

            la = ra - 1;

            // 更新装甲板数据 i
            // 读取大小装甲阈值
            this->armor.inputparam(this->light_left, this->light_right, armor_index);
            this->armor_box.emplace_back(this->armor);

            // 画图
            if (this->armor_config.DRAW_ARMOR_RECT == 1 &&
                this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                Point2f vtx[4];
                this->armor.returnRect().points(vtx);
                for (int j = 0; j < 4; ++j) {
                    line(this->armor_img, vtx[j], vtx[(j + 1) % 4], Scalar(150, 200, 0), 2, 8, 0);
                }

                Point put_armor_angle = Point(this->armor.returnRect().center.x,
                                              this->armor.returnRect().center.y - 20);
                putText(armor_img, to_string(this->armor.returnAngle()), put_armor_angle,
                        FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
                Point put_size = Point(this->armor.returnRect().center.x,
                                       this->armor.returnRect().center.y - 30);
                putText(armor_img, to_string(this->armor.returnAngle()), put_armor_angle,
                        FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
                Point put_aspect_ratio = Point(this->armor.returnRect().center.x,
                                               this->armor.returnRect().center.y - 50);
                putText(armor_img, to_string(this->armor.returnAspectRatio()), put_aspect_ratio,
                        FONT_HERSHEY_PLAIN, 1, Scalar(120, 255, 0), 1, 8, false);

                switch (this->armor.returnArmorType()) {
                    case SMALL_ARMOR:
                        putText(armor_img, "Small", put_size, FONT_HERSHEY_PLAIN, 1,
                                Scalar(0, 255, 0), 1, 8, false);
                        break;
                    case BIG_ARMOR:
                        putText(armor_img, "Big", put_size, FONT_HERSHEY_PLAIN, 1,
                                Scalar(0, 255, 0), 1, 8, false);
                        break;
                    default:
                        putText(armor_img, "Small", put_size, FONT_HERSHEY_PLAIN, 1,
                                Scalar(0, 255, 0), 1, 8, false);
                        break;
                }
            }

            armor_index += 1;
            over_cnt = 0;
        }
        else {
            // 匹配失败继续匹配下一个
            over_cnt += 1;
            if (over_cnt == 1) {
                include_armor_index = la;
                is_include_armor    = true;
            }
            --la;
        }

        //设置范围，根据参数设定
        // 判断范围是否超出参数设定

        if (over_cnt > this->armor_config.candidate_armor_cfg.MATCHING_TH|| diff_light_y <= light_right.height*0.5 || (la < 0 && ra > 1) ||
            (ra == 1 && la < 0)) {
            // 超出范围重置窗口
            --ra;
            la               = ra - 1;
            over_cnt         = 0;
            is_include_armor = false;
        }
    }

    // 显示装甲板图像
    if (this->armor_config.DRAW_ARMOR_RECT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("armor", this->armor_img);
        cout << "🧐 装甲板 框选图像已打开 🧐" << endl;
    }

    this->light_box.clear();
    vector<Light>(this->light_box).swap(this->light_box);


    if (this->armor_box.size() < 1) {
        // 无目标，更新标志位为false，计数器++，退出，重置数据
        this->is_last_data_catch = false;
        this->is_Lost_target     = true;  // 丢失目标
        this->lose_target_cnt += 1;
        return false;
    }

        // 有目标，更新标志位为true，计数器清空，重置数据
        this->is_last_data_catch = true;  // 检测到装甲板，则下一帧会标识上一帧有数据
        this->is_Lost_target  = false;  // 未丢失目标
        this->lose_target_cnt = 0;

    return true;
}

/**
 * @brief 选取最优装甲板
 */
void RM_Armor::armorSelectOptimum()
{
    this->is_curract_armor = false;
    if (this->armor_box.size() == 1) {
        // 若只有一个，直接当做最优解，提前退出
        // TODO:测试另外一个版本的
        // this->final_armor = armor_box.back();
        // this->last_armor  = this->final_armor;
        // is_curract_armor  = true;

        for (auto iter = armor_box.begin(); iter != armor_box.end(); ++iter) {
            // 储存最优装甲板
            this->final_armor = (*iter);
            this->last_armor  = this->final_armor;
            is_curract_armor  = true;
        }
    }
    else {
        // 多个的时候根据两个方法进行筛选test：TODO
        // 方法一：先按照中心距进行排序，再根据一定斜率阈值进行选择
        // 方法二：先根据斜率进行排序，再根据中心点距离逐个排查斜率的排序按照一定条件来进行最终的选择

        // 循环遍历armor_box ，先根据距离进行排序方法一：离中心点越近，数字越小
        sort(this->armor_box.begin(), this->armor_box.end(),
             [](CandidateArmor& c1, CandidateArmor& c2) {
                 return c1.returnDistToCenter() <
                        c2.returnDistToCenter();  // 按中心距进行排序，越小越前
                 //  return c1.returnAngle() < c2.returnAngle();  // 按斜率进行排序，越小越前
             });

        int i                    = 0;
        this->aspest_ratio_min   = 9999.f;
        this->dist_to_center_min = 9999999999.f;
        // 中点可以通过candidatearmor来读 TODO
        for (auto iter = armor_box.begin(); iter != armor_box.end(); ++iter) {
            (*iter).updateIndex(i);
            // TODO 加上roi的左上角距离
            // if ((*iter).returnDistToCenter() <= this->dist_to_center_min) {
            //     this->dist_to_center_min = (*iter).returnDistToCenter();
            if (fabs((*iter).returnRect().angle - 180.f) <= aspest_ratio_min) {
                this->aspest_ratio_min = fabs((*iter).returnRect().angle - 180.f);

                this->final_armor = (*iter);
                is_curract_armor  = true;
            }

            //显示排序结果
            if (this->armor_config.DRAW_ARMOR_RECT == 1 &&
                this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                Point put_index =
                    Point((*iter).returnRect().center.x, (*iter).returnRect().center.y - 30);
                putText(this->dst_img, to_string((*iter).returnIndex()), put_index,
                        FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
                cout << "🧐 装甲板 排序后编号已打开 🧐" << endl;
            }

            if (i == 2) {
                break;
            }

            i += 1;
        }
        // 得到最终值，退出循环
    }

    // 防跳动 记得加上roi的tl
    if (centerDistance(this->final_armor.returnRect().center,
                       this->last_armor.returnRect().center) > 10) {
        this->final_armor = this->armor_box.front();
    }
    this->last_armor = this->final_armor;

    if (this->is_curract_armor) {
        if (this->armor_config.DRAW_ARMOR_RECT == 1 &&
            this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
            Point2f vtx[4];
            this->final_armor.returnRect().points(vtx);
            for (int j = 0; j < 4; ++j) {
                line(this->dst_img, vtx[j], vtx[(j + 1) % 4], Scalar(0, 255, 0), 2, 8, 0);
            }
        }
    }
    // 对最终目标进行顶点排序
    this->sortArmorVertex();

    // 显示装甲板图像
    if (this->armor_config.DRAW_ARMOR_RECT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("final_armor", this->dst_img);
        cout << "🧐 最优装甲板 框选图像已打开 🧐" << endl;
    }

    this->armor_box.clear();
    vector<CandidateArmor>(this->armor_box).swap(this->armor_box);

}

void RM_Armor::sortArmorVertex()
{
    Point2f lu, ld, ru, rd;

    // 二、拟合灯条外顶点
    Point2f vertex_l[4];
    Point2f vertex_r[4];

    this->light_left.rect.points(vertex_l);
    this->light_right.rect.points(vertex_r);

    sort(vertex_l,vertex_l+4,[](const Point2f& p1,const Point2f& p2){return p1.y< p2.y;});
    sort(vertex_r,vertex_r+4,[](const Point2f& p1,const Point2f& p2){return p1.y< p2.y;});

    if(vertex_l[0].x<vertex_l[1].x){
         lu = vertex_l[0];
        ld = vertex_l[2];
    }
    else{
        lu = vertex_l[1];
        ld = vertex_l[3];
    }
    if (vertex_r[2].y < vertex_r[3].y)	{
        ru = vertex_r[1];
        rd = vertex_r[3];
    }
    else {
        ru = vertex_r[0];
        rd = vertex_r[2];
    }

    // 一 拟合装甲板顶点
    // Point2f vertex[4];
    // this->armor.returnRect().points(vertex);

    // sort(vertex, vertex + 4, [](const Point2f& p1, const Point2f& p2) { return p1.x < p2.x; });

    // if (vertex[0].y < vertex[1].y){
    //     lu = vertex[0];
    //     ld = vertex[1];
    // }
    // else{
    //     lu = vertex[1];
    //     ld = vertex[0];
    // }
    // if (vertex[2].y < vertex[3].y)	{
    //     ru = vertex[2];
    //     rd = vertex[3];
    // }
    // else {
    //     ru = vertex[3];
    //     rd = vertex[2];
    // }

    Point put_lu = Point(lu.x, lu.y - 20);
    putText(dst_img, "0", put_lu, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
    Point put_ru = Point(ru.x, ru.y - 20);
    putText(dst_img, "1", put_ru, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
    Point put_rd = Point(rd.x, rd.y - 20);
    putText(dst_img, "2", put_rd, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
    Point put_ld = Point(ld.x, ld.y - 20);
    putText(dst_img, "3", put_ld, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);

    // 二 左右灯条外顶点

    target_2d.clear();
    target_2d.emplace_back(lu);
    target_2d.emplace_back(ru);
    target_2d.emplace_back(rd);
    target_2d.emplace_back(ld);
}

vector<Point2f> RM_Armor::returnTarget2DPoint(){
    return this->target_2d;
}

CandidateArmor& RM_Armor::returnFinalArmor(){
    return this->final_armor;
}


float centerDistance(const Point& p1, const Point& p2)
{
    float D =
        static_cast<float>(sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y))));
    return D;
}

