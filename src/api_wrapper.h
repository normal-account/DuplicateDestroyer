//
// Created by carle on 3/13/22.
//
#include <cpr/cpr.h>

#define USER_AGENT "DuplicateDestroyer 1.2v C++"
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

   cpr::Response fetch_token();

   void set_token( const std::string &newToken );

   void set_time_expire ( unsigned long long unixTime );

   unsigned long long get_time_expire();

   cpr::Response fetch_messages();

   void download_image( const std::string &url );

   cpr::Response submit_comment( const std::string &content, const std::string &id );

   void save_submission( const std::string &id );

   void report_submission( const std::string &id );

   void accept_invite( const std::string &subreddit );

   void mark_message_as_read( const std::string &fullname );
};