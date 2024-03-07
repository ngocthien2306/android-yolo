#include "yolo_cls.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <string>
using namespace std;

ImageClassifier::ImageClassifier() {}

void ImageClassifier::loadModel(AAssetManager* mgr, const char* modeltype, int _target_size) {
    net.opt.use_vulkan_compute = false;

    char parampath[256];
    char modelpath[256];

    sprintf(parampath, "model.ncnn_s.param", modeltype);
    sprintf(modelpath, "model.ncnn_s.bin", modeltype);

    net.load_param(mgr, parampath);
    net.load_model(mgr, modelpath);

    targetSize = _target_size;
}

void ImageClassifier::preprocessImage(const cv::Mat& bgr, ncnn::Mat& inPad) {
    inPad = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR2RGB, bgr.cols, bgr.rows, targetSize, targetSize);

    const float normVals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    inPad.substract_mean_normalize(0, normVals);
}

pair<int, pair<string, float>> ImageClassifier::inference(const cv::Mat& image) {
    ncnn::Mat inPad;
    preprocessImage(image, inPad);

    ncnn::Extractor ex = net.create_extractor();
    ex.input("in0", inPad);

    ncnn::Mat out;
    ex.extract("out0", out);

    out = out.reshape(out.h * out.w * out.c);
    vector<float> predictedScores(out.w);

    for (int j = 0; j < out.w; ++j) {
        predictedScores[j] = out[j];
    }

    int maxIndex = static_cast<int>(std::max_element(predictedScores.begin(), predictedScores.end()) - predictedScores.begin());
    string className = classLabels[maxIndex];

    return make_pair(maxIndex, make_pair(className, predictedScores[maxIndex]));
}

void ImageClassifier::loadLabels(AAssetManager* mgr, const std::string& labelPath) {
    AAsset* asset = AAssetManager_open(mgr, labelPath.c_str(), AASSET_MODE_BUFFER);
    if (asset == nullptr) {
        cerr << "Error opening label asset: " << labelPath << endl;
        exit(-1);
    }

    const void* assetBuffer = AAsset_getBuffer(asset);
    off_t assetLength = AAsset_getLength(asset);
    std::string labelContent(static_cast<const char*>(assetBuffer), assetLength);

    // Split the content into lines
    istringstream labelStream(labelContent);
    string line;
    while (getline(labelStream, line)) {
        classLabels.push_back(line);
    }

    AAsset_close(asset);
}
