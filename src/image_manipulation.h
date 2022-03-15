// NOTE : A class is not needed here. We always manipulate the same file, only its content changes, so creating
// different objects for each downloaded image would be futile.

#include <iostream>
#include <opencv2/opencv.hpp>

#define IMAGE_NAME "image_in_process"

using namespace cv;
using namespace std;

class Image {
public:
   Mat matrix;

   string hash10x10();
   string hash8x8();
};
