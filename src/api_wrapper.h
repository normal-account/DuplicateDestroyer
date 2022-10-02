//
// Created by carle on 3/13/22.
//
#include <cpr/cpr.h>

#define USER_AGENT "Linux:DuplicateDestroyer:v2.0 C++ (by /u/PowerModerator)"
#define IMAGE_NAME "image_in_process"

class ApiWrapper
{
   std::string token;
   unsigned long long timeToExpire = 0;

public:
   void send_message( const std::string &user, const std::string &content, const std::string &subject );

   void remove_submission( const std::string &id );

   cpr::Response fetch_submissions();

   cpr::Response fetch_top_submissions(const std::string &sub, const std::string &range);

   static cpr::Response fetch_token();

   void set_token( const std::string &newToken );

   void set_time_expire ( unsigned long long unixTime );

   unsigned long long get_time_expire() const;

   cpr::Response fetch_messages();

   static void download_image( const std::string &url );

   cpr::Response submit_comment( const std::string &content, const std::string &id );

   //void save_submission( const std::string &id );

   void report_submission( const std::string &id, const std::string &reason );

   void accept_invite( const std::string &subreddit );

   void mark_message_as_read( const std::string &fullname );

   void distinguish_comment( const std::string &fullname);

   cpr::Response subreddit_exists(const std::string &sub);
};