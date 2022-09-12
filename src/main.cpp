#include "api_wrapper.h"
#include "image_manipulation.h"
#include "reddit_entities.h"
#include "string_constants.h"
#include <chrono>
tesseract::TessBaseAPI* tessBaseApi;
ApiWrapper apiWrapper;
db_interface interface( (Session("localhost", 33060, "db_user", "soleil")));

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

    // Or PSM_AUTO_OSD for extra info (OSD). Difference not significant.
    // See : https://pyimagesearch.com/2021/11/15/tesseract-page-segmentation-modes-psms-explained-how-to-improve-your-ocr-accuracy/
    tessBaseApi->SetVariable("debug_file", "/dev/null");
    tessBaseApi->SetPageSegMode( tesseract::PSM_AUTO);   //matches -psm 1 from the command line
    atexit((destroyTesseract));
}

std::string create_markdown_header(const std::string &author, const std::string &date) {
    return "**OP:** " + author + "\n\n" + "**Date:** " + date + "\n\n**Duplicates:**\n\n" + MARKDOWN_TABLE;
}

std::string create_markdown_row(int number, int similarity, Row &row) {
    auto id = row.get(3).get<std::string>();
    auto author = row.get(4).get<std::string>();
    auto dimensions = row.get(5).get<std::string>();
    auto unixDate = row.get(6).get<u_long>();
    auto date = std::chrono::sys_seconds(std::chrono::seconds(unixDate));
    //std::cout << date << std::endl;
    auto title = row.get(8).get<std::string>();
    auto url = row.get(9).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + std::to_string(unixDate)
        + " | " + std::to_string(unixDate) + " | " + "[" + std::to_string(similarity) + "%](" + url + ") | "
        + dimensions + " | [" + title + "](https://redd.it/" + id + ")";
}

bool search_duplicates(Submission &submission, SubredditSetting &settings, Image &image, bool eightPxHash, std::string &commentContent) {

    mpz_class hash;
    auto query = interface.get_image_rows();

    if (eightPxHash)
        image.computeHash8x8();
    else
    {
        image . computeHash10x10();
        commentContent.append(HEADER_REMOVE_IMAGE);
    }

    commentContent.append(create_markdown_header(submission.author, std::to_string(submission.created)));
    std::string imageOcrString = image . get_text();

    int numberDuplicates = 0;

    for (auto row : *query) {
        std::string strhash =  eightPxHash ? row.get(2).get<std::string>() : row.get(1).get<std::string>();
        mpz_class mpzhash;
        mpzhash.set_str(strhash, 10);
        int similarity = eightPxHash ? image.compareHash8x8(mpzhash) : image.compareHash10x10( mpzhash );
        if (similarity > settings.report_threshold &&
        image.get_string_similarity(row.get(0).get<std::string>(), imageOcrString) > settings.ocr_text_threshold) {
            numberDuplicates++;
            commentContent.append(create_markdown_row( numberDuplicates, similarity, row));
        }
    }

    if (!eightPxHash)
        commentContent.append(FOOTER_REMOVE_IMAGE);

    return numberDuplicates > 0;
}

void process_image(Image &image, const std::string &url) {
    apiWrapper.download_image(url);
    image.matrix = imread(IMAGE_NAME);
    image.extract_text();
}


void handle_image(Submission &submission, SubredditSetting &settings, const std::string &url) {
    Image image;
    std::string commentContent;
    process_image(image, url);
    bool submissionRemoved = search_duplicates(submission, settings, image, true, commentContent);
    if (submissionRemoved) {
        apiWrapper.remove_submission(submission.fullname);
        apiWrapper . submit_comment( commentContent, submission . fullname );
    } else {
        submissionRemoved = search_duplicates(submission, settings, image, false, commentContent);
        if (submissionRemoved)
        {
            apiWrapper . submit_comment( commentContent, submission . fullname );
            apiWrapper . report_submission( submission . fullname );
        } else
            interface.insert_submission(image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, "", submission.created, submission.isVideo, submission.title, url);
    }
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
        interface.switch_subreddit(submission.subreddit);
        RowResult settingsQuery = interface.get_subreddit_settings(submission.subreddit);
        SubredditSetting settings(settingsQuery);


        std::cout << submission.id << std::endl;

        if (submission.isGallery) {
            for (const auto& url : submission.galleryUrls) {
                handle_image(submission, settings, url);
            }
        }
        else {
            handle_image(submission, settings, submission.url);
        }
    }


    exit(0);
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
