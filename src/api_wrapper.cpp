//
// Created by carle on 3/13/22.
//
#include <iostream>
#include "api_wrapper.h"

std::string token;

// Should be done only once the original token expires.
void refresh_token() {
    cpr::Response refresh_query = cpr::Post(cpr::Url{"https://www.reddit.com/api/v1/access_token"},
                                            cpr::Authentication{"6_T2X8heZAm6sRvBLADkwQ", "ka6iiWVZDZKbAjYWao_0h5lLjWdYNw"},
                                            cpr::Parameters{{"grant_type", "refresh_token"}, {"refresh_token", token}},
                                            cpr::VerifySsl(0),
                                            cpr::UserAgent(USER_AGENT)
    );

    std::cout << refresh_query.status_code << std::endl;
    std::cout << refresh_query.text << std::endl;
}

void unsave_submission()
{
    cpr::Response save_query = cpr::Post( cpr::Url{"https://oauth.reddit.com/api/unsave"},
                                          cpr::Header{{"Authorization", token}},
                                          cpr::Parameters{{"id", "t3_hivul7"},},
                                          cpr::VerifySsl( 0 ),
                                          cpr::UserAgent( USER_AGENT ));
}

cpr::Response fetchSubmissions() {
    return cpr::Get(cpr::Url{"https://oauth.reddit.com/r/abruptchaos/new"},
                                        cpr::Header{{"Authorization", token}},
                                        cpr::Parameters{{"g", "GLOBAL"}, {"limit", "1"}},
                                        cpr::VerifySsl(0),
                                        cpr::UserAgent(USER_AGENT));
}


cpr::Response fetchMessages() {
    return cpr::Get(cpr::Url{"https://oauth.reddit.com/message/inbox"}, // TODO: change to unread
                    cpr::Header{{"Authorization", token}},
                    cpr::Parameters{{"mark", "true"}, {"limit", "2"}},
                    cpr::VerifySsl(0),
                    cpr::UserAgent(USER_AGENT));
}


cpr::Response fetchImage( std::string url) {
    return cpr::Get(cpr::Url{"https://i.redd.it/xi16okzxbdn81.jpg"},
                                           cpr::VerifySsl(0),
                                           cpr::UserAgent(USER_AGENT));
}


cpr::Response fetchToken() {
    cpr::Response access_query = cpr::Post(cpr::Url{"https://www.reddit.com/api/v1/access_token"},
                                           cpr::Authentication{"6_T2X8heZAm6sRvBLADkwQ", "ka6iiWVZDZKbAjYWao_0h5lLjWdYNw"},
                                           cpr::Parameters{{"grant_type", "password"}, {"username", "Im_a_Necrophiliac"},  {"password", "soleil"}, {"code", "8P8dRN4wS299Vy-K3_177uO7t8DIMw#_"}},
                                           cpr::VerifySsl(0),
                                           cpr::UserAgent(USER_AGENT)
    );
    return access_query;
}

void setToken(std::string newToken) {
    token = newToken;
}