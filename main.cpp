#include <iostream>
#include "api.h"
#include "entities.h"

using namespace std;

// Should be done only once the original token expires.
void initializeToken()
{
    cpr::Response tokenQuery = fetchToken();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    setToken( ogToken );
}

int main()
{
    // TODO : Set up DB lib
    // TODO : Set up mailing
    // TODO : Set up algorithm to create hash

    initializeToken();

    cpr::Response messageQuery = fetc

    cpr::Response submissionQuery = fetchSubmissions();
    //std::cout << submissionQuery.header["X-Ratelimit-Remaining"] << std::endl;
    std::cout << submissionQuery . text << std::endl;

    json jsonEntry = json::parse( submissionQuery . text );

    Submission submission;
    json submissionList = jsonEntry[ "data" ][ "children" ];

    for ( auto submissionIter = submissionList.begin(); submissionIter != submissionList.end(); submissionIter++)
    {
        submission << submissionIter.value()[ "data" ];
        cout << submission . shortlink << " " << submission . title << " " << submission . url << endl;
    }




    return 0;
}
