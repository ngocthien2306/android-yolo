#pragma once

#include <vector>
#include <string>
#include <opencv2/core/core.hpp>
#include "net.h"

class ImageClassifier {
public:
    ImageClassifier();
    void loadModel(AAssetManager* mgr, const char* modeltype, int target_size);
    void loadLabels(AAssetManager* mgr, const std::string& labelPath);

    std::pair<int,  std::pair<std::string, float>> inference(const cv::Mat& image);
    std::vector<std::string> classLabels;


private:
    ncnn::Net net;
    int targetSize;
    void preprocessImage(const cv::Mat& bgr, ncnn::Mat& inPad);
};
