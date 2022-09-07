
#include "../single_include/nlohmann/json.hpp"
#include <iostream>
#include <optional>
using namespace std;

using json = nlohmann::json;

class Submission
{
public:
   string author;
   string title;
   string shortlink;
   string fullname;
   string id;
   bool isVideo;
   bool isGallery;
   int score;

   string url;
   vector<string> galleryUrls;

    // To consider a modified version of the Levenshtein algorithm that takes string length in consideration
   int getEditDistance(const std::string &first, const std::string& second);

   // Useful for OCR text and title comparison
   double findStringSimilarity(const std::string &first, const std::string &second);

   void operator<<( json data );
};

class Message
{
public:
   bool isReply; // a message is either a reply or a private message
   string subject;
   string body;
   string author;

   void operator<<( json data );
};

class SubredditSetting
{

};


