//
// Created by carle on 10/8/22.
//

#include "comment_formatting.h"

std::string dimensions_to_string(cv::Size dimensions) {
    return std::to_string(dimensions.width) + "x" + std::to_string(dimensions.height);
}

std::string get_time_interval(long long int ogTime, long long int newTime) {
    long long int diff = newTime - ogTime;
    std::string stringDiff;
    if (diff > 31104000) {
        stringDiff = std::to_string(diff / 31104000) + " year(s) before";
    } else if (diff > 2592000) {
        stringDiff = std::to_string(diff / 2592000) + " month(s) before";
    } else if (diff > 86400) {
        stringDiff = std::to_string(diff / 86400) + " day(s) before";
    } else if (diff > 3600) {
        stringDiff = std::to_string(diff / 3600) + " hour(s) before";
    } else if (diff > 60) {
        stringDiff = std::to_string(diff / 60) + " minute(s) before";
    } else if (diff > 0) {
        stringDiff = std::to_string(diff) + " second(s) before";
    } else
        stringDiff = "at the same time";
    return stringDiff;
}

int get_days_interval(long long int ogTime, long long int newTime) {
    long long int diff = newTime - ogTime;
    return (int)(diff / 86400);
}

std::string unix_time_to_string(time_t unixTime) {
    struct tm *tm = localtime(&unixTime);
    char charDate[30];
    strftime(charDate, sizeof(charDate), "%Y-%m-%d %H:%M:%S", tm);
    std::string strDate(charDate);
    return strDate;
}

std::string create_image_markdown_header( const std::string &author, const std::string &date, const std::string &dimensions) {
    return "**OP:** " + author + "\n\n**Date:** " + date + "\n\n**Dimensions:** " + dimensions + "\n\n**Duplicates:**\n\n" + MARKDOWN_TABLE_IMAGE;
}

std::string create_markdown_header( const std::string &author, const std::string &date, const std::string &MARKDOWN_HEADER) {
    return "**OP:** " + author + "\n\n**Date:** " + date + "\n\n**Duplicates:**\n\n" + MARKDOWN_HEADER;
}

std::string create_image_markdown_row( int number, int similarity, Row row, long long submissionTime, int textSimilarity) {
    auto id = row.get(DB_ID).get<std::string>();
    auto author = row.get(DB_AUTHOR).get<std::string>();
    auto dimensions = row.get(DB_DIMENSIONS).get<std::string>();

    auto unixDate = row.get(DB_DATE).get<time_t>();

    auto title = row.get(DB_TITLE).get<std::string>();
    auto url = row.get(DB_URL).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + unix_time_to_string(unixDate)
           + " | " + get_time_interval(unixDate, submissionTime) + " | " + "[" + std::to_string(similarity) + "%](" + url + ") | "
           + (textSimilarity == -1 ?  "N/A" : (std::to_string(textSimilarity) + "%") ) + " | " + dimensions + " | [" + title + "](https://redd.it/" + id + ")\n";
}


std::string create_link_markdown_row( int number, Row &row, long long submissionTime) {
    auto author = row.get(DB_AUTHOR).get<std::string>();
    auto id = row.get(DB_ID).get<std::string>();
    auto unixDate = row.get(DB_DATE).get<time_t>();
    auto title = row.get(DB_TITLE).get<std::string>();
    auto url = row.get(DB_URL).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + unix_time_to_string(unixDate)
           + " | " + get_time_interval(unixDate, submissionTime) + " | [url](https://" + url + ") | [" + title + "](https://redd.it/" + id + ")\n";
}

std::string create_title_markdown_row( int number, Row &row, long long submissionTime, int similarity) {
    auto author = row.get(DB_AUTHOR).get<std::string>();
    auto id = row.get(DB_ID).get<std::string>();
    auto unixDate = row.get(DB_DATE).get<time_t>();
    auto title = row.get(DB_TITLE).get<std::string>();
    auto url = row.get(DB_URL).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + unix_time_to_string(unixDate)
           + " | " + get_time_interval(unixDate, submissionTime) + "| [" +
           std::to_string(similarity) + "%](https://" + url + ") | [" + title + "](https://redd.it/" + id + ")\n";
}

std::string get_modmail_footer(const std::string &sub, const std::string &context) {
    std::string modmailContent;
    modmailContent.append(MODMAIL_FOOTER_1).append(sub).append(MODMAIL_FOOTER_2).append(context).append(MODMAIL_FOOTER_3);
    return modmailContent;
}