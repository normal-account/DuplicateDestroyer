
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
   bool isVideo;
   bool isGallery;
   string url;
   int score;
   optional<vector<string>> galleryUrls;

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


