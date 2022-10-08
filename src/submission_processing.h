#include <iostream>
#include <cpr/cpr.h>
#include "api_wrapper.h"
#include "image_manipulation.h"
#include "reddit_entities.h"
#include "api_wrapper.h"
#include "db_interface.h"
#include "comment_formatting.h"

#ifndef DUPLICATEDESTROYER_SUBMISSION_PROCESSING_H
#define DUPLICATEDESTROYER_SUBMISSION_PROCESSING_H

extern std::vector<std::thread*> benchmarkThreads;
extern std::binary_semaphore setSemaphore;
extern ApiWrapper apiWrapper;
extern db_interface *interfaces[NUMBER_THREADS + 1];
extern tesseract::TessBaseAPI* tessBaseApi[NUMBER_THREADS];

std::string get_comment_fullname(const cpr::Response &query);

std::string submit_comment(const std::string &content, const std::string &fullname);

// Returns true if the described submission fits the criterias for removal
bool determine_remove(int imageSimilarity, int imageThreshold, double textSimilarity, int textLength1, int textLength2);

// Returns true if the described submission fits the criterias for reporting
bool determine_report(int imageSimilarity, int imageThreshold, double textSimilarity, int textLength1, int textLength2);

void post_action_comment( std::string commentContent, const std::vector<std::string> &removeComment, const std::vector<std::string> &reportComment,
                          const Submission &submission, const std::string &HEADER, bool reportReplies);

bool search_image_duplicates(const Submission &submission, const SubredditSetting &settings, Image &image, int threadNumber);

void process_image(Image &image, const std::string &url, int threadNumber);

bool handle_image(const Submission &submission, const SubredditSetting &settings, Image &image, const std::string &url, int threadNumber);

bool handle_link(const Submission &submission, const SubredditSetting &settings, int threadNumber);

bool handle_title(const Submission &submission, const SubredditSetting &settings, int threadNumber);

void import_submissions(const std::string &subreddit, int threadNumber);

void insert_submission(const Submission &submission, Image &image, bool isMedia, int threadNumber);

void process_submission(bool *finished, std::set<std::string> *sub2thread, std::set<std::string> *requiredOrder, const Submission &submission, const SubredditSetting &settings, int threadNumber);

// Choosing the next thread in which to process a submission of the subreddit corresponding to 'sub'
[[nodiscard]] int choose_thread(const std::string &sub, const bool finished[NUMBER_THREADS], std::set<std::string> &sub2thread, std::set<std::string> &requiredOrder);

inline void next_iteration(_List_iterator<Submission> &it, std::list<Submission> &list);

void populate_list(std::list<Submission> &submissionList, json &submissionJSON);

void iterate_submissions();


#endif //DUPLICATEDESTROYER_SUBMISSION_PROCESSING_H
