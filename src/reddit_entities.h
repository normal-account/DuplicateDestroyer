
#include "../single_include/nlohmann/json.hpp"
#include <iostream>
#include <optional>
#include "db_interface.h"

using namespace std;

using namespace ::mysqlx;
using json = nlohmann::json;

enum TYPE {
   IMAGE,
   VIDEO,
   LINK,
   SELFTEXT
};

class Submission
{
public:
   explicit Submission(json data);

   std::string subreddit;
   std::string author;
   std::string title;
   std::string shortlink;
   std::string fullname;
   std::string id;
   long long int created;

   TYPE type;
   bool isGallery;
   int score;


   std::string url;
   vector<std::string> galleryUrls;

   [[nodiscard]] bool is_video() const;
   [[nodiscard]] bool is_image() const;
};

class Message
{
public:
   explicit Message(json data);
   bool isReply; // a message is either a reply or a private message
   std::string subject;
   std::string body;
   std::string fullname;
   std::optional<std::string> author;
   std::optional<std::string> subreddit;
   std::string type;
   std::string id;
   std::optional<std::string> context;
};

class SubredditSetting
{
public:
    explicit SubredditSetting(RowResult &settings);

    bool enabled;
    bool imported;
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


