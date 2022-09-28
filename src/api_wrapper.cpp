#include <iostream>
#include "api_wrapper.h"

#define HEADERS cpr::Header{{"Authorization", token}}, cpr::VerifySsl(false), cpr::UserAgent(USER_AGENT))
#define HANDLE_STATUS if (query.status_code != 200) throw
#define HANDLE_RATELIMIT if (stoi(query.header["X-Ratelimit-Remaining"]) == 0) { sleep(stoi(query.header["x-ratelimit-reset"])); std::cerr << "SLEPT" << std::endl; }




void ApiWrapper::save_submission( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/save"},
                                     cpr::Parameters{{"id", fullname},},
    HEADERS;
    HANDLE_STATUS;
    HANDLE_RATELIMIT;
}

void ApiWrapper::send_message( const std::string &user, const std::string &content, const std::string &subject )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/compose"},
                                          cpr::Parameters{{"subject", subject},
                                                          {"text",    content},
                                                          {"to",      user}},
    HEADERS;
    HANDLE_STATUS;
    HANDLE_RATELIMIT;
}


void ApiWrapper::mark_message_as_read( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/read_message"},
                                          cpr::Parameters{{"id", fullname}},
    HEADERS;
    HANDLE_STATUS;
    HANDLE_RATELIMIT;
}



void ApiWrapper::remove_submission( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/remove"},
                                     cpr::Parameters{{"id",   fullname},
                                                     {"spam", "false"}},
    HEADERS;
    HANDLE_STATUS;
    HANDLE_RATELIMIT;
}

void ApiWrapper::report_submission( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/report"},
                                     cpr::Parameters{{"thing_id", fullname},
                                                     {"reason", "Found duplicate(s) - check pinned comment."},
                                                     {"api_type", "json"},
                                                     {"custom_text", "true"}},
    HEADERS;
    HANDLE_STATUS;
    HANDLE_RATELIMIT;
}


cpr::Response ApiWrapper::fetch_submissions()
{
    auto query = cpr::Get( cpr::Url{"https://oauth.reddit.com/r/memes_benchmarks_dd/new"},
                     cpr::Parameters{{"g",     "GLOBAL"},
                                     {"limit", "200"}},
    HEADERS;
    HANDLE_STATUS;
    return query;
}

cpr::Response ApiWrapper::fetch_top_submissions(const std::string &sub, const std::string &range)
{
    auto query = cpr::Get( cpr::Url{"https://oauth.reddit.com/r/ForCSSTesting/top"},
                           cpr::Parameters{{"g",     "GLOBAL"},
                                           {"limit", "100"},
                                           {"t", range}},
    HEADERS;
    HANDLE_STATUS;
    return query;
}


cpr::Response ApiWrapper::fetch_messages()
{
    auto query = cpr::Get( cpr::Url{"https://oauth.reddit.com/message/unread"}, // TODO: change to unread
                     cpr::Parameters{{"mark",  "true"},
                                     {"limit", "1"}},
    HEADERS;
    HANDLE_STATUS;
    return query;
}

cpr::Response ApiWrapper::submit_comment( const std::string &content, const std::string &id )
{
    auto query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/comment"},
                            cpr::Parameters{{"api_type",      "json"},
                                            {"text",          content},
                                            {"thing_id",      id},
                                            {"return_rtjson", "false"}},
    HEADERS;

    HANDLE_STATUS;
    HANDLE_RATELIMIT;

    return query;
}

void ApiWrapper::distinguish_comment( const std::string &fullname )
{
    auto query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/distinguish"},
                            cpr::Parameters{{"api_type",      "json"},
                                            {"how",      "yes"},
                                            {"id",      fullname},
                                            {"sticky", "true"}},
    HEADERS;
    HANDLE_STATUS;
    HANDLE_RATELIMIT;
}

void ApiWrapper::accept_invite( const std::string &subreddit )
{
    auto query = cpr::Post( cpr::Url{"https://oauth.reddit.com" + subreddit + "/api/accept_moderator_invite"},
                            cpr::Parameters{{"api_type", "json"}},
    HEADERS;
    HANDLE_STATUS;
    HANDLE_RATELIMIT;
}


void ApiWrapper::download_image( const std::string &url )
{

    cpr::Response query = cpr::Get( cpr::Url{url},
                                         cpr::VerifySsl( false ),
                                         cpr::UserAgent( USER_AGENT ));

    HANDLE_STATUS;
    std::ofstream tempFile( IMAGE_NAME );
    tempFile << query . text;
}


cpr::Response ApiWrapper::fetch_token()
{
    cpr::Response query = cpr::Post( cpr::Url{"https://www.reddit.com/api/v1/access_token"},
                                            cpr::Authentication{"6_T2X8heZAm6sRvBLADkwQ",
                                                                "ka6iiWVZDZKbAjYWao_0h5lLjWdYNw"},
                                            cpr::Parameters{{"grant_type", "password"},
                                                            {"username",   "Im_a_Necrophiliac"},
                                                            {"password",   "soleil"},
                                                            },
                                            cpr::VerifySsl( 0 ),
                                            cpr::UserAgent( USER_AGENT )
    );
    HANDLE_STATUS;
    HANDLE_RATELIMIT;

    return query;
}

cpr::Response ApiWrapper::subreddit_exists( const std::string &sub )
{
    auto query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/search_subreddits"},
                            cpr::Parameters{{"exact", "false"},
                                            {"include_over_18", "true"},
                                            {"include_unadvertisable", "true"},
                                            {"query", sub}},
    HEADERS;

    HANDLE_STATUS;
    HANDLE_RATELIMIT;
    return query;
}

void ApiWrapper::set_token( const std::string &newToken )
{
    token = newToken ;
}

void ApiWrapper::set_time_expire( unsigned long long unixTime )
{
    timeToExpire = unixTime;
}

unsigned long long ApiWrapper::get_time_expire()
{
    return timeToExpire;
}


