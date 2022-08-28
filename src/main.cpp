#include "api_wrapper.h"
#include "reddit_entities.h"
#include "image_manipulation.h"
#include <mysqlx/xdevapi.h>

using namespace ::mysqlx;

// Should be done only once the original token expires.
void initializeToken()
{
    cpr::Response tokenQuery = fetchToken();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    setToken( ogToken );
}

inline bool downloadImage(std::string url) {

    cpr::Response imageQuery = fetchImage( url );

    if (imageQuery.status_code == 200) {
        ofstream tempFile(IMAGE_NAME);
        tempFile << imageQuery.text;
        return true;
    }
    return false;
}



// TODO : Set up DB lib
//std::cout << submissionQuerySTR.header["X-Ratelimit-Remaining"] << std::endl;
int main()
{
    initializeToken();

    /*Image image;
    image.matrix = imread("test.jpg");*/

    /* <-- SUBMISSIONS --> */
    // Check status of mysqld service for further details
    /*Session sess("localhost", 33060, "db_user", "soleil");
    Schema db= sess.getSchema("all_reposts");
    std::cout << db.getTables().begin().operator*().select().execute().begin().operator*().get(0) << std::endl;*/


    cpr::Response submissionQuery = fetchSubmissions();

    json submissionList = json::parse( submissionQuery . text )[ "data" ][ "children" ];
    Submission submission;
    for ( auto submissionIter = submissionList.begin(); submissionIter != submissionList.end(); submissionIter++)
    {
        std::cout << submissionIter.value()[ "data" ] << std::endl;
        submission << submissionIter.value()[ "data" ];
        cout << submission . shortlink << " " << submission . title << " " << submission . url << endl;
    }

    exit(0);

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
