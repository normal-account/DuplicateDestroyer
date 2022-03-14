#include "api.h"
#include "entities.h"
#include <opencv2/opencv.hpp>

using namespace cv;
#define IMAGE_NAME "image_in_process"

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
// TODO : Set up algorithm to create hash
//std::cout << submissionQuerySTR.header["X-Ratelimit-Remaining"] << std::endl;
int main()
{
    initializeToken();

    Mat image;
    image = imread( "test.jpg", 1 );
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);
    waitKey(0);
    exit(0);

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
