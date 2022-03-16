#include "api_wrapper.h"
#include "reddit_entities.h"
#include "image_manipulation.h"

// Should be done only once the original token expires.
void initializeToken()
{
    cpr::Response tokenQuery = fetchToken();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    setToken( ogToken );
}

bool downloadImage(string url) {

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

    cpr::Response submissionQuery = fetchSubmissions();

    json submissionList = json::parse( submissionQuery . text )[ "data" ][ "children" ];
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
