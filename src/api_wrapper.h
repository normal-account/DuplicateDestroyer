//
// Created by carle on 3/13/22.
//
#include <cpr/cpr.h>

#define USER_AGENT "Linux:DuplicateDestroyer:v2.0 C++ (by /u/PowerModerator)"
//#define IMAGE_NAME "image_in_process"

class ApiWrapper
{
   int NUMBER_THREADS;
   std::shared_ptr<cpr::Session> *sessions;
   std::string token;
   unsigned long long timeToExpire = 0;

public:
   explicit ApiWrapper(int numberThreads) : NUMBER_THREADS(numberThreads)
   {
       sessions = new std::shared_ptr<cpr::Session>[NUMBER_THREADS];
       for (int i = 0; i < NUMBER_THREADS; i++) {
           sessions[i] = std::make_shared<cpr::Session>();
       }
   }

   ~ApiWrapper() {
       delete sessions;
   }

   void send_message( const std::string &user, const std::string &content, const std::string &subject );

   void remove_submission( const std::string &id );

   cpr::Response fetch_submissions();

   cpr::Response fetch_top_submissions(const std::string &sub, const std::string &range);

   cpr::Response fetch_token(int threadNumber);

   void set_token( const std::string &newToken );

   void set_time_expire ( unsigned long long unixTime );

   [[nodiscard]] unsigned long long get_time_expire() const;

   cpr::Response fetch_messages();

   static void download_image( const std::string &url, const std::string &imageName );

   cpr::Response submit_comment( const std::string &content, const std::string &id );

   void report_submission( const std::string &id, const std::string &reason );

   void accept_invite( const std::string &subreddit );

   void mark_message_as_read( const std::string &fullname );

   void distinguish_comment( const std::string &fullname);

   cpr::Response subreddit_exists(const std::string &sub);
};