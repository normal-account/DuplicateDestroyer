//
// Created by carle on 3/13/22.
//
#include <iostream>
#include <utility>
#include "api_wrapper.h"

#define HEADERS cpr::Header{{"Authorization", token}}, cpr::VerifySsl(false), cpr::UserAgent(USER_AGENT))

// TODO : Handle errors gracefully for each method. Maybe use Exceptions ?
// Should be done only once the original token expires.
void ApiWrapper::refresh_token()
{
    cpr::Response refresh_query = cpr::Post( cpr::Url{"https://www.reddit.com/api/v1/access_token"},
                                             cpr::Authentication{"6_T2X8heZAm6sRvBLADkwQ",
                                                                 "ka6iiWVZDZKbAjYWao_0h5lLjWdYNw"},
                                             cpr::Parameters{{"grant_type",    "refresh_token"},
                                                             {"refresh_token", token}},
                                             cpr::VerifySsl( false ),
                                             cpr::UserAgent( USER_AGENT )
    );

}

void ApiWrapper::unsave_submission( const std::string &id )
{
    cpr::Response save_query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/unsave"},
                                          cpr::Parameters{{"id", "t3_hivul7"},},
    HEADERS;
}

void ApiWrapper::send_message( const std::string &user, const std::string &content, const std::string &subject )
{
    cpr::Response save_query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/compose"},
                                          cpr::Parameters{{"subject", subject},
                                                          {"text",    content},
                                                          {"to",      user}},
    HEADERS;
}

void ApiWrapper::remove_submission( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/remove"},
                                     cpr::Parameters{{"id",   fullname},
                                                     {"spam", "false"}},
    HEADERS;

}

void ApiWrapper::report_submission( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/report"},
                                     cpr::Parameters{{"id", fullname},
                                                     {"reason", "false"},
                                                     {"api_type", "json"}},
    HEADERS;
}


cpr::Response ApiWrapper::fetch_submissions()
{
    return cpr::Get( cpr::Url{"https://oauth.reddit.com/r/ForCSSTesting/new"},
                     cpr::Parameters{{"g",     "GLOBAL"},
                                     {"limit", "1"}},
    HEADERS;
}


cpr::Response ApiWrapper::fetch_messages()
{
    return cpr::Get( cpr::Url{"https://oauth.reddit.com/message/inbox"}, // TODO: change to unread
                     cpr::Parameters{{"mark",  "true"},
                                     {"limit", "2"}},
    HEADERS;
}

void ApiWrapper::submit_comment( const std::string &content, const std::string &id )
{
    auto query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/comment"},
                            cpr::Parameters{{"api_type",      "json"},
                                            {"text",          content},
                                            {"thing_id",      id},
                                            {"return_rtjson", "false"}},
    HEADERS;
    // TODO: Return comment id
    std::cout << query.text << std::endl;
}


void ApiWrapper::accept_invite( const std::string &subreddit )
{
    auto query = cpr::Post( cpr::Url{"https://oauth.reddit.com/r/" + subreddit + "/api/accept_moderator_invite"},
                            cpr::Parameters{{"api_type", "json"}},
    HEADERS;
}


bool ApiWrapper::download_image( const std::string &url )
{

    cpr::Response imageQuery = cpr::Get( cpr::Url{url},
                                         cpr::VerifySsl( false ),
                                         cpr::UserAgent( USER_AGENT ));

    if ( imageQuery . status_code == 200 )
    {
        std::ofstream tempFile( IMAGE_NAME );
        tempFile << imageQuery . text;
        return true;
    }
    return false;
}


cpr::Response ApiWrapper::fetch_token()
{
    cpr::Response access_query = cpr::Post( cpr::Url{"https://www.reddit.com/api/v1/access_token"},
                                            cpr::Authentication{"6_T2X8heZAm6sRvBLADkwQ",
                                                                "ka6iiWVZDZKbAjYWao_0h5lLjWdYNw"},
                                            cpr::Parameters{{"grant_type", "password"},
                                                            {"username",   "Im_a_Necrophiliac"},
                                                            {"password",   "soleil"},
                                                            {"code",       "8P8dRN4wS299Vy-K3_177uO7t8DIMw#_"}},
                                            cpr::VerifySsl( 0 ),
                                            cpr::UserAgent( USER_AGENT )
    );
    return access_query;
}

void ApiWrapper::set_token( std::string newToken )
{
    token = std::move( newToken );
}