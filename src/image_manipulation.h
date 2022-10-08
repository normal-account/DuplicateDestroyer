// NOTE : A class is not needed here. We always manipulate the same file, only its content changes, so creating
// different objects for each downloaded image would be futile.

#include <iostream>
#include <opencv2/opencv.hpp>
#include <gmpxx.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include "string_constants.h"

#ifndef DUPLICATEDESTROYER_IMAGE_MANIPULATION_H
#define DUPLICATEDESTROYER_IMAGE_MANIPULATION_H

using namespace cv;
using namespace std;

double get_string_similarity(const std::string &first, const std::string &second);
int getEditDistance(const std::string &first, const std::string& second);

class Image {
public:
   Mat matrix;
   std::string ocrText;

   static std::string determine_image_name(int threadNumber);
   static int compareHash(const mpz_class &hash1, const mpz_class &hash2);
   int compareHash10x10(const mpz_class& hash);
   int compareHash8x8(const mpz_class& hash);
   void computeHash10x10();
   void computeHash8x8();
   [[nodiscard]] mpz_class getHash10x10() { return hash10x10; };
   [[nodiscard]] mpz_class getHash8x8() { return hash8x8; }
   // Used for OCR text and title comparison
   void extract_text(int threadNumber);
   [[nodiscard]] static inline std::string prepare_word(std::string &word);
   [[nodiscard]] std::string get_text() const { return ocrText; }
   [[nodiscard]] std::string filter_non_words(const std::set<std::string> &dict) const;
   [[nodiscard]] cv::Size get_dimensions() const;


private:
   mpz_class hash10x10;
   mpz_class hash8x8;
};

#endif // DUPLICATEDESTROYER_IMAGE_MANIPULATION_H
