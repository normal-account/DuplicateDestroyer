#include "api_wrapper.h"
#include "image_manipulation.h"
#include "reddit_entities.h"

tesseract::TessBaseAPI* tessBaseApi;
ApiWrapper apiWrapper;
db_interface interface("localhost", 33060, "db_user", "soleil");

using namespace ::mysqlx;

// Should be done only once the original token expires.
void initializeToken()
{
    cpr::Response tokenQuery = apiWrapper.fetch_token();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    apiWrapper.set_token( ogToken );
}

void destroyTesseract() {
    tessBaseApi->End();
    delete tessBaseApi;
}

void initializeTesseract()
{
    tessBaseApi = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (tessBaseApi->Init( nullptr, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Or PSM_AUTO_OSD for extra info (OSD). Difference not significiant.
    // See : https://pyimagesearch.com/2021/11/15/tesseract-page-segmentation-modes-psms-explained-how-to-improve-your-ocr-accuracy/
    tessBaseApi->SetPageSegMode( tesseract::PSM_AUTO);   //matches -psm 1 from the command line
    atexit((destroyTesseract));
}

bool search_duplicates(Submission &submission, SubredditSetting &settings, bool eightPxHash) {
    Image image;
    apiWrapper.download_image(submission.url);
    image.matrix = imread(IMAGE_NAME);
    auto ocrStringsQuery = interface.get_ocr_strings();
    std::shared_ptr<std::vector<mpz_class>> hashesQuery;
    mpz_class hash;

    if (eightPxHash) {
        image.computeHash8x8();
        hashesQuery = interface.get_8x8_hashes();
    }
    else {
        image.computeHash10x10();
        hashesQuery = interface.get_10x10_hashes();
    }

    auto ocrStrings = ocrStringsQuery->begin();
    auto hashes = hashesQuery->begin();
    bool submissionRemoved = false;
    for (; ocrStrings != ocrStringsQuery->end() && hashes != hashesQuery->end(); ocrStrings++, hashes++) {
        int similarity =  eightPxHash ? image.compareHash8x8(*hashes) : image.compareHash10x10(*hashes);
        std::string imageOcrString = image . extract_text();
        if (similarity > settings.report_threshold &&
            image.get_string_similarity(*ocrStrings, imageOcrString) > settings.ocr_text_threshold) {
            apiWrapper.remove_submission(submission.id);
            submissionRemoved = true;
        }
    }
    return submissionRemoved;
}

//std::cout << submissionQuerySTR.header["X-Ratelimit-Remaining"] << std::endl;
int main()
{
    initializeToken();
    initializeTesseract();


    /* <-- SUBMISSIONS --> */
    // Check status of mysqld service for further details

    /*
    auto table = db.getTable("test");
    auto query = table.select("name").where("name is not null");
    std::cout << query.execute().begin().operator*().get(0) << std::endl;*/
    //std::cout << db.getTables().begin().operator*().select().execute().begin().operator*().get(0) << std::endl;

    cpr::Response submissionQuery = apiWrapper.fetch_submissions();

    json submissionList = json::parse( submissionQuery . text )[ "data" ][ "children" ];

    for ( auto submissionIter = submissionList.begin(); submissionIter != submissionList.end(); submissionIter++)
    {
        Submission submission(submissionIter.value()[ "data" ]);
        auto settingsQuery = interface.get_subreddit_settings(submission.subreddit);
        SubredditSetting settings(settingsQuery);

        cout << submission . shortlink << " " << submission . title << " " << submission . url << endl;

        if (submission.isGallery) {
            for (const auto& url : submission.galleryUrls) {
                search_duplicates(submission, settings, true);
            }
        }
        else {
            bool submissionRemoved = search_duplicates(submission, settings, true);
            if (!submissionRemoved) {
                search_duplicates(submission, settings, false);
            }
        }
    }



    //downloadImage(submission.url);
    /*Image image;
    image.matrix = imread("test.jpg");*/


    /* <-- MESSAGES --> */
    cpr::Response messageQuery = apiWrapper.fetch_messages();

    json messageList = json::parse( messageQuery.text )[ "data" ][ "children" ];
    for ( auto messageIter = messageList.begin(); messageIter != messageList.end(); messageIter++) {
        Message message(messageIter.value());
        if (!message.isReply)
            cout << message.author << " " << message.subject << endl;
    }

    return 0;
}
