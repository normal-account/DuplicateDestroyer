#include "image_manipulation.h"

string Image::hash10x10() {
    Mat matrix10px;

    cv::resize(matrix, matrix10px, {10, 10});
    cvtColor(matrix10px, matrix10px, cv::COLOR_BGR2GRAY);

    // Note : at gets row number, then column number
    unsigned char previousPixel = matrix10px.at<unsigned char>(0, 9);
    unsigned long long differenceHash = 0;
    for (int row = 0; row < 10; row += 2) {
        for (int col = 0; col < 10; col++) {

        }
    }
    imwrite("test2.jpg", matrix10px);
    return "";
}

string Image::hash8x8() {
    Mat matrix8px;
    cv::resize(matrix, matrix8px, {8, 8});

    return "";
}
