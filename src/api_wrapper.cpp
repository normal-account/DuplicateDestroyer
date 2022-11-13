#include <iostream>
#include "api_wrapper.h"

#define HEADERS cpr::Header{{"Authorization", token}}, cpr::VerifySsl(false), cpr::UserAgent(USER_AGENT))
#define HANDLE_STATUS(X) if (query.status_code != 200) throw std::runtime_error(X + query.text)
#define HANDLE_RATELIMIT if (stoi(query.header["X-Ratelimit-Remaining"]) < NUMBER_THREADS) { sleep(stoi(query.header["x-ratelimit-reset"])); std::cerr << "SLEPT" << std::endl; }

void ApiWrapper::send_message( const std::string &user, const std::string &content, const std::string &subject )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/compose"},
                                          cpr::Parameters{{"subject", subject},
                                                          {"text",    content},
                                                          {"to",      user}},
    HEADERS;
    HANDLE_STATUS("send");
    HANDLE_RATELIMIT;
}


void ApiWrapper::mark_message_as_read( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/read_message"},
                                          cpr::Parameters{{"id", fullname}},
    HEADERS;
    HANDLE_STATUS("read_msg");
    HANDLE_RATELIMIT;
}


void ApiWrapper::remove_submission( const std::string &fullname )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/remove"},
                                     cpr::Parameters{{"id",   fullname},
                                                     {"spam", "false"}},
    HEADERS;
    HANDLE_STATUS("remove");
    HANDLE_RATELIMIT;
}

void ApiWrapper::report_submission( const std::string &fullname, const std::string &reason )
{
    cpr::Response query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/report"},
                                     cpr::Parameters{{"thing_id", fullname},
                                                     {"reason", reason},
                                                     {"api_type", "json"},
                                                     {"custom_text", "true"}},
    HEADERS;
    HANDLE_STATUS("report");
    HANDLE_RATELIMIT;
}


cpr::Response ApiWrapper::fetch_submissions()
{
    auto query = cpr::Get( cpr::Url{"https://oauth.reddit.com/r/mod/new"},
                     cpr::Parameters{{"g",     "GLOBAL"},
                                     {"limit", "100"}},
    HEADERS;
    HANDLE_STATUS("fetch_submissions");
    std::cout << "LEFT : " << stoi(query.header["X-Ratelimit-Remaining"]) << " and reset in " << stoi(query.header["x-ratelimit-reset"]) << std::endl; // TODO: REMOVE
    return query;
}

cpr::Response ApiWrapper::fetch_top_submissions(const std::string &sub, const std::string &range)
{
    auto query = cpr::Get( cpr::Url{"https://oauth.reddit.com/r/" + sub + "/top"},
                           cpr::Parameters{{"g",     "GLOBAL"},
                                           {"limit", "100"},
                                           {"t", range}},
    HEADERS;
    HANDLE_STATUS("fetch_top");
    return query;
}


cpr::Response ApiWrapper::fetch_messages()
{
    auto query = cpr::Get( cpr::Url{"https://oauth.reddit.com/message/unread"},
                     cpr::Parameters{{"mark",  "true"},
                                     {"limit", "10"}},
    HEADERS;
    HANDLE_STATUS("fetch_msg");
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
    HANDLE_STATUS("submit");
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
    HANDLE_STATUS("distinguish");
    HANDLE_RATELIMIT;
}


void ApiWrapper::accept_invite( const std::string &subreddit )
{
    auto query = cpr::Post( cpr::Url{"https://oauth.reddit.com" + subreddit + "/api/accept_moderator_invite"},
                            cpr::Parameters{{"api_type", "json"}},
    HEADERS;
    HANDLE_STATUS("accept_invite");
    HANDLE_RATELIMIT;
}


void ApiWrapper::download_image( const std::string &url, const std::string &imageName )
{
    cpr::Response query = cpr::Get( cpr::Url{url},
                                         cpr::VerifySsl( false ),
                                         cpr::UserAgent( USER_AGENT ));

    HANDLE_STATUS("download " + url);
    std::ofstream tempFile( imageName );
    tempFile << query . text;
}

bool ApiWrapper::image_deleted( const std::string &url )
{
    cpr::Response query = cpr::Get( cpr::Url{url},
                                    cpr::VerifySsl( false ),
                                    cpr::UserAgent( USER_AGENT ));

    return query.status_code == 404;
}


cpr::Response ApiWrapper::fetch_token() const
{
    auto query = cpr::Post(cpr::Url{"https://www.reddit.com/api/v1/access_token"},
                           cpr::Authentication{"7FqDgsrKsu5IYL5N53ZVZg", "pP2bDfRkr_vGaAhBfYE-CyzuLt2Tlw",cpr::AuthMode::BASIC
                           },
                           cpr::Parameters{{"grant_type", "password"},
                                           {"username",   "dd_testing_account"},
                                           {"password",   "soleil100"},
                           },
                           cpr::VerifySsl( 0 ),
                           cpr::UserAgent( USER_AGENT ));
    HANDLE_STATUS("fetch_token");
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

    HANDLE_STATUS("subreddit_exists");
    HANDLE_RATELIMIT;
    return query;
}

void ApiWrapper::set_token( const std::string &newToken )
{
    token = newToken;
}

void ApiWrapper::set_time_expire( unsigned long long unixTime )
{
    timeToExpire = unixTime;
}

unsigned long long ApiWrapper::get_time_expire() const
{
    return timeToExpire;
}


