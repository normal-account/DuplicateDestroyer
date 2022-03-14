
#include "single_include/nlohmann/json.hpp"

using json = nlohmann::json;

class Submission
{
public:
   std::string author;
   std::string title;
   std::string shortlink;
   std::string fullname;
   bool isVideo;
   std::string url;
   int score;

   void operator<<( json data );
};

class Message
{
public:

};


