#include "api_wrapper.h"
#include "reddit_entities.h"
#include "image_manipulation.h"
#include "db_interface.h"

tesseract::TessBaseAPI* api;

// Should be done only once the original token expires.
void initializeToken()
{
    cpr::Response tokenQuery = fetch_token();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    set_token( ogToken );
}

void destroyTesseract() {
    api->End();
    delete api;
}

void initializeTesseract()
{
    api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Or PSM_AUTO_OSD for extra info (OSD). Difference not significiant.
    // See : https://pyimagesearch.com/2021/11/15/tesseract-page-segmentation-modes-psms-explained-how-to-improve-your-ocr-accuracy/
    api->SetPageSegMode(tesseract::PSM_AUTO);   //matches -psm 1 from the command line
    atexit((destroyTesseract));
}



// TODO : Set up DB lib
//std::cout << submissionQuerySTR.header["X-Ratelimit-Remaining"] << std::endl;
int main()
{
    initializeToken();
    initializeTesseract();


    db_interface interface("localhost", 33060, "db_user", "soleil");

    /* <-- SUBMISSIONS --> */
    // Check status of mysqld service for further details

    /*
    auto table = db.getTable("test");
    auto query = table.select("name").where("name is not null");
    std::cout << query.execute().begin().operator*().get(0) << std::endl;*/
    //std::cout << db.getTables().begin().operator*().select().execute().begin().operator*().get(0) << std::endl;


    cpr::Response submissionQuery = fetch_submissions();

    json submissionList = json::parse( submissionQuery . text )[ "data" ][ "children" ];
    Submission submission;
    for ( auto submissionIter = submissionList.begin(); submissionIter != submissionList.end(); submissionIter++)
    {
        submission << submissionIter.value()[ "data" ];
        cout << submission . shortlink << " " << submission . title << " " << submission . url << endl;

    }

    Image image;
    if (submission.isGallery) {
        for (const auto& url : submission.galleryUrls) {
            downloadImage(url, IMAGE_NAME);
            image.matrix = imread(IMAGE_NAME);
        }
    }
    else {
        downloadImage(submission.url, IMAGE_NAME);
        image.matrix = imread(IMAGE_NAME);
        image.computeHash8x8();
        for (const auto& dbHash : *interface.get_8x8_hashes()) {
            int similarity = image.compareHash8x8(dbHash);

        }
    }

    //downloadImage(submission.url);
    /*Image image;
    image.matrix = imread("test.jpg");*/


    /* <-- MESSAGES --> */
    cpr::Response messageQuery = fetch_messages();

    json messageList = json::parse( messageQuery.text )[ "data" ][ "children" ];
    Message message;
    for ( auto messageIter = messageList.begin(); messageIter != messageList.end(); messageIter++) {
        message << messageIter.value();
        if (!message.isReply)
            cout << message.author << " " << message.subject << endl;
    }

    return 0;

    destroyTesseract();
}
