//
// Created by carle on 3/13/22.
//
#include <cpr/cpr.h>

#define USER_AGENT "DuplicateDestroyer 1.2v C++"
#define IMAGE_NAME "image_in_process"

class ApiWrapper
{
   std::string token;

public:
   void send_message( const std::string &user, const std::string &content, const std::string &subject );

   void remove_submission( const std::string &id );

   cpr::Response fetch_submissions();

   cpr::Response fetch_token();

   void set_token( std::string newToken );

   void refresh_token();

   cpr::Response fetch_messages();

   bool download_image( const std::string &url );

   void submit_removal_comment( const std::string &id );

   void unsave_submission( const std::string &id );
};