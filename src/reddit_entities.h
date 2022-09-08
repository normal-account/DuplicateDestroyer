
#include "../single_include/nlohmann/json.hpp"
#include <iostream>
#include <optional>
#include "db_interface.h"

using namespace std;

using namespace ::mysqlx;
using json = nlohmann::json;

class Submission
{
public:
   Submission(json data);

   std::string subreddit;
   std::string author;
   std::string title;
   std::string shortlink;
   std::string fullname;
   std::string id;
   bool isVideo;
   bool isGallery;
   int score;

   std::string url;
   vector<std::string> galleryUrls;


};

class Message
{
public:
   explicit Message(json data);
   bool isReply; // a message is either a reply or a private message
   std::string subject;
   std::string body;
   std::string author;
};

class SubredditSetting
{
public:
    explicit SubredditSetting(const mysqlx::internal::Iterator<mysqlx::internal::Row_result_detail<mysqlx::abi2::r0::Columns>, mysqlx::Row>& settings);

    bool enabled;
    int remove_threshold;
    int report_threshold;
    bool enforce_videos;
    bool enforce_images;
    bool enforce_links;
    int time_range;
    bool report_links;
    bool report_replies;
    bool enforce_titles;
    int title_remove_threshold;
    int title_report_threshold;
    int min_title_length_to_enforce;
    int removal_table_duplicate_number;
    int ocr_text_threshold;

};


