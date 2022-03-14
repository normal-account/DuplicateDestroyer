//
// Created by carle on 3/13/22.
//
#include <cpr/cpr.h>

#define USER_AGENT "DuplicateDestroyer 1.2v C++"


cpr::Response fetchSubmissions();

cpr::Response fetchToken();

void setToken(std::string newToken);

void refresh_token();

cpr::Response fetchMessages();

cpr::Response fetchImage( std::string url);