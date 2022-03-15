
#include "../single_include/nlohmann/json.hpp"
#include <iostream>
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
   string url;
   int score;

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


