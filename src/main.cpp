#include "api_wrapper.h"
#include "image_manipulation.h"
#include "reddit_entities.h"
#include "string_constants.h"

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

std::string create_markdown_header(const std::string &author, const std::string &date) {
    return "**OP:** " + author + "\n\n" + "**Date:** " + date + "\n\n**Duplicates:**\n\n" + MARKDOWN_TABLE;
}

std::string create_markdown_row(int number, int similarity, mysqlx::internal::Iterator<mysqlx::internal::Row_result_detail<mysqlx::abi2::r0::Columns>, mysqlx::Row> row) {
    auto id = (*row).get(3).get<std::string>();
    auto author = (*row).get(4).get<std::string>();
    auto dimensions = (*row).get(5).get<std::string>();
    auto date = (*row).get(6).get<long long int>();
    auto title = (*row).get(8).get<std::string>();
    auto url = (*row).get(9).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + std::to_string(date)
        + " | " + std::to_string(date) + " | " + "[" + std::to_string(similarity) + "%](" + url + ") | "
        + dimensions + " | [" + title + "](https://redd.it/" + id + ")";
}

bool search_duplicates(Submission &submission, SubredditSetting &settings, bool eightPxHash) {
    Image image;
    apiWrapper.download_image(submission.url);
    image.matrix = imread(IMAGE_NAME);
    auto ocrStringsQuery = interface.get_ocr_strings();
    mpz_class hash;
    auto query = interface.get_image_rows();
    std::string comment_content;

    if (eightPxHash)
        image.computeHash8x8();
    else
    {
        image . computeHash10x10();
        comment_content.append(HEADER_REMOVE_IMAGE);
    }

    comment_content.append(create_markdown_header(submission.author, std::to_string(submission.created)));
    std::string imageOcrString = image . extract_text();
    auto ocrStrings = ocrStringsQuery->begin();
    auto row = query->begin();
    int number = 0;

    for (; ocrStrings != ocrStringsQuery->end() && row != query->end(); ocrStrings++, ++row) {
        int similarity = eightPxHash ? image.compareHash8x8((*row).get(2).get<mpz_class>() ) : image.compareHash10x10( (*row).get(1).get<mpz_class>()  );
        if (similarity > settings.report_threshold &&
        image.get_string_similarity(*ocrStrings, imageOcrString) > settings.ocr_text_threshold) {
            number++;
            comment_content.append(create_markdown_row(number, similarity, row));
            apiWrapper.remove_submission(submission.id);
        }
    }

    if (!eightPxHash)
        comment_content.append(FOOTER_REMOVE_IMAGE);

    if (number > 0)
        interface.insert_submission(imageOcrString, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, "", 10, submission.isVideo, submission.title);

    apiWrapper.submit_removal_comment(comment_content, submission.id);

    return number > 0;
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
        std::cout << submissionIter.value()[ "data" ] << std::endl;
        Submission submission(submissionIter.value()[ "data" ]);
        auto settingsQuery = interface.get_subreddit_settings(submission.subreddit);
        SubredditSetting settings(settingsQuery);

        cout << submission . shortlink << " " << submission . title << " " << submission . url << endl;

        if (submission.isGallery) {
            for (const auto& url : submission.galleryUrls) {
                bool submissionRemoved = search_duplicates(submission, settings, true);
                if (!submissionRemoved) {
                    search_duplicates(submission, settings, false);
                }
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
