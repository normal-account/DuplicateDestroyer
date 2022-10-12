#include <iostream>
#include <mysqlx/xdevapi.h>
#include <opencv2/opencv.hpp>
#include "string_constants.h"

#ifndef DUPLICATEDESTROYER_COMMENT_FORMATTING_H
#define DUPLICATEDESTROYER_COMMENT_FORMATTING_H

using namespace ::mysqlx;

std::string dimensions_to_string(cv::Size dimensions);

std::string get_time_interval(long long int ogTime, long long int newTime);

int get_days_interval(long long int ogTime, long long int newTime);

std::string unix_time_to_string(time_t unixTime);

std::string create_image_markdown_header( const std::string &author, const std::string &date, const std::string &dimensions);

std::string create_markdown_header( const std::string &author, const std::string &date, const std::string &MARKDOWN_HEADER);

std::string create_image_markdown_row( int number, int similarity, Row row, long long submissionTime, int textSimilarity);

std::string create_link_markdown_row( int number, Row &row, long long submissionTime);

std::string create_title_markdown_row( int number, Row &row, long long submissionTime, int similarity);

// Forms the footer inviting users to report a false positive
std::string get_modmail_footer(const std::string &sub, const std::string &context);

#endif //DUPLICATEDESTROYER_COMMENT_FORMATTING_H
