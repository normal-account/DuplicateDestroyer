#include "api.h"
#include "entities.h"


// Should be done only once the original token expires.
void initializeToken()
{
    cpr::Response tokenQuery = fetchToken();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    setToken( ogToken );
}
// TODO : Set up DB lib
// TODO : Set up mailing
// TODO : Set up algorithm to create hash
//std::cout << submissionQuerySTR.header["X-Ratelimit-Remaining"] << std::endl;

int main()
{
    initializeToken();




    /* <-- SUBMISSIONS --> */

    cpr::Response submissionQuerySTR = fetchSubmissions();

    json submissionList = json::parse( submissionQuerySTR . text )[ "data" ][ "children" ];
    Submission submission;
    for ( auto submissionIter = submissionList.begin(); submissionIter != submissionList.end(); submissionIter++)
    {
        submission << submissionIter.value()[ "data" ];
        cout << submission . shortlink << " " << submission . title << " " << submission . url << endl;
    }


    /* <-- MESSAGES --> */
    cpr::Response messageQuery = fetchMessages();

    json messageList = json::parse( messageQuery.text )[ "data" ][ "children" ];
    Message message;
    for ( auto messageIter = messageList.begin(); messageIter != messageList.end(); messageIter++) {
        message << messageIter.value();
        if (!message.isReply)
            cout << message.author << " " << message.subject << endl;
    }




    return 0;
}
