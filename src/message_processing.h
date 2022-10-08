#include <iostream>
#include "reddit_entities.h"
#include "db_interface.h"
#include "string_constants.h"
#include "api_wrapper.h"

#ifndef DUPLICATEDESTROYER_MESSAGE_PROCESSING_H
#define DUPLICATEDESTROYER_MESSAGE_PROCESSING_H

extern ApiWrapper apiWrapper;
extern db_interface *interfaces[NUMBER_THREADS + 1];


bool is_number(const std::string& s);

std::string bool_row(Row &row, int pos);

std::string int_row(Row &row, int pos);

std::string get_subreddit_settings_list(const std::string &sub, int threadNumber);

bool subreddit_exists(const std::string &sub);

std::string get_last_word(std::string &s);

void parse_subreddit_settings(const Message &message, int threadNumber);

void iterate_messages(int threadNumber=0);

#endif //DUPLICATEDESTROYER_MESSAGE_PROCESSING_H
