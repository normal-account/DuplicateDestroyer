// NOTE : A class is not needed here. We always manipulate the same file, only its content changes, so creating
// different objects for each downloaded image would be futile.

#include <iostream>
#include <opencv2/opencv.hpp>
#include <gmpxx.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#define IMAGE_NAME "image_in_process"

using namespace cv;
using namespace std;

class Image {
public:
   Mat matrix;
   std::string ocrText;

   int compareHash(const mpz_class &hash1, const mpz_class &hash2);
   int compareHash10x10(const mpz_class& hash);
   int compareHash8x8(const mpz_class& hash);
   void computeHash10x10();
   void computeHash8x8();
   [[nodiscard]] mpz_class getHash10x10() { return hash10x10; };
   [[nodiscard]] mpz_class getHash8x8() { return hash8x8; }
   // Used for OCR text and title comparison
   double get_string_similarity(const std::string &first, const std::string &second);
   void extract_text();
   [[nodiscard]] std::string get_text() const { return ocrText; }

private:
   // To consider a modified version of the Levenshtein algorithm that takes string length in consideration
   int getEditDistance(const std::string &first, const std::string& second);


   mpz_class hash10x10;
   mpz_class hash8x8;
};
