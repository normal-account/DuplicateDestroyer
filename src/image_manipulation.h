// NOTE : A class is not needed here. We always manipulate the same file, only its content changes, so creating
// different objects for each downloaded image would be futile.

#include <iostream>
#include <opencv2/opencv.hpp>
#include <gmpxx.h>

#define IMAGE_NAME "image_in_process"

using namespace cv;
using namespace std;

class Image {
public:
   Mat matrix;

   mpz_class hash10x10();
   mpz_class hash8x8();
};
