#include "api_wrapper.h"
#include "image_manipulation.h"
#include "reddit_entities.h"

tesseract::TessBaseAPI* api;
using namespace ::mysqlx;

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

    for ( auto submissionIter = submissionList.begin(); submissionIter != submissionList.end(); submissionIter++)
    {
        Submission submission(submissionIter.value()[ "data" ]);
        auto settingsQuery = interface.get_subreddit_settings(submission.subreddit);
        SubredditSetting settings(settingsQuery);

        cout << submission . shortlink << " " << submission . title << " " << submission . url << endl;

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
            auto ocrStringsQuery = interface.get_ocr_strings();
            auto hashesQuery = interface.get_8x8_hashes();
            auto ocrStrings = ocrStringsQuery->begin();
            auto hashes = hashesQuery->begin();
            bool submissionRemoved = false;
            for (; ocrStrings != ocrStringsQuery->end() && hashes != hashesQuery->end(); ocrStrings++, hashes++) {
                int similarity = image.compareHash8x8(*hashes);
                std::string imageOcrString = image.extractTextFromImage();
                if (similarity > settings.report_threshold &&
                image.get_string_similarity(*ocrStrings, imageOcrString) > settings.ocr_text_threshold) {
                    remove_submission(submission.id);
                    submissionRemoved = true;
                }
            }

            if (!submissionRemoved) {
                image.computeHash8x8();
                ocrStringsQuery = interface.get_ocr_strings();
                hashesQuery = interface.get_8x8_hashes();
                ocrStrings = ocrStringsQuery->begin();
                hashes = hashesQuery->begin();
                submissionRemoved = false;
                for (; ocrStrings != ocrStringsQuery->end() && hashes != hashesQuery->end(); ocrStrings++, hashes++) {
                    int similarity = image.compareHash8x8(*hashes);
                    std::string imageOcrString = image.extractTextFromImage();
                    if (similarity > settings.remove_threshold &&
                        image.get_string_similarity(*ocrStrings, imageOcrString) > settings.ocr_text_threshold) {
                        remove_submission(submission.id);
                        submissionRemoved = true;
                    }
                }
            }
        }
    }



    //downloadImage(submission.url);
    /*Image image;
    image.matrix = imread("test.jpg");*/


    /* <-- MESSAGES --> */
    cpr::Response messageQuery = fetch_messages();

    json messageList = json::parse( messageQuery.text )[ "data" ][ "children" ];
    for ( auto messageIter = messageList.begin(); messageIter != messageList.end(); messageIter++) {
        Message message(messageIter.value());
        if (!message.isReply)
            cout << message.author << " " << message.subject << endl;
    }

    return 0;

    destroyTesseract();
}
