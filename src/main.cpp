#include "api_wrapper.h"
#include "image_manipulation.h"
#include "reddit_entities.h"
#include "string_constants.h"
#include <chrono>
#include <cstdio>

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

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string dimensions_to_string(cv::Size dimensions) {
    return std::to_string(dimensions.width) + "x" + std::to_string(dimensions.height);
}

std::string create_markdown_header(const std::string &author, const std::string &date, const std::string &dimensions) {
    return "**OP:** " + author + "\n\n**Date:** " + date + "\n\n**Dimensions:** " + dimensions + "\n\n**Duplicates:**\n\n" + MARKDOWN_TABLE;
}

std::string get_time_interval(long long int ogTime, long long int newTime) {
    long long int diff = newTime - ogTime;
    std::string stringDiff;
    if (diff > 31104000) {
        stringDiff = std::to_string(diff / 31104000) + " year(s) before";
    } else if (diff > 2592000) {
        stringDiff = std::to_string(diff / 2592000) + " month(s) before";
    } else if (diff > 86400) {
        stringDiff = std::to_string(diff / 86400) + " day(s) before";
    } else if (diff > 3600) {
        stringDiff = std::to_string(diff / 3600) + " hour(s) before";
    } else if (diff > 60) {
        stringDiff = std::to_string(diff / 60) + " minute(s) before";
    } else if (diff > 0) {
        stringDiff = std::to_string(diff) + " second(s) before";
    } else
        stringDiff = "at the same time";
    return stringDiff;
}

int get_days_interval(long long int ogTime, long long int newTime) {
    long long int diff = newTime - ogTime;
    return (int)(diff / 86400);
}

std::string unix_time_to_string(time_t unixTime) {
    struct tm *tm = localtime(&unixTime);
    char charDate[30];
    strftime(charDate, sizeof(charDate), "%Y-%m-%d %H:%M:%S", tm);
    std::string strDate(charDate);
    return strDate;
}

std::string create_markdown_row(int number, int similarity, Row &row, long long submissionTime) {
    auto id = row.get(DB_ID).get<std::string>();
    auto author = row.get(DB_AUTHOR).get<std::string>();
    auto dimensions = row.get(DB_DIMENSIONS).get<std::string>();

    auto unixDate = row.get(DB_DATE).get<time_t>();

    auto title = row.get(DB_TITLE).get<std::string>();
    auto url = row.get(DB_URL).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + unix_time_to_string(unixDate)
        + " | " + get_time_interval(unixDate, submissionTime) + " | " + "[" + std::to_string(similarity) + "%](https://" + url + ") | "
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

    commentContent.append(create_markdown_header(submission.author, unix_time_to_string(submission.created), dimensions_to_string(image.get_dimensions())));
    std::string imageOcrString = image . get_text();

    int numberDuplicates = 0;

    for (auto row : *query) {
        if (numberDuplicates > settings.removal_table_duplicate_number)
            break;

        if (get_days_interval(row.get(DB_DATE).get<u_long>(), submission.created) > settings.time_range)
            continue;

        std::string strhash =  eightPxHash ? row.get(DB_8PXHASH).get<std::string>() : row.get(DB_10PXHASH).get<std::string>(); // get 10px hash or 8px one
        mpz_class mpzhash;
        mpzhash.set_str(strhash, 10);
        int similarity = eightPxHash ? image.compareHash8x8(mpzhash) : image.compareHash10x10( mpzhash );
        if (similarity > (eightPxHash ? settings.report_threshold : settings.remove_threshold) &&
        image.get_string_similarity(row.get(DB_OCRSTRING).get<std::string>(), imageOcrString) > settings.ocr_text_threshold) {
            numberDuplicates++;
            commentContent.append(create_markdown_row( numberDuplicates, similarity, row, submission.created ));
        }
    }

    if (!eightPxHash)
        commentContent.append(FOOTER_REMOVE_IMAGE);

    return numberDuplicates > 0;
}

std::string get_last_word(std::string &s) {
    auto index = s.find_last_of(' ');
    std::string last_word = s.substr(++index);
    return last_word;
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
        apiWrapper . submit_comment( commentContent, submission . fullname );
        apiWrapper . remove_submission(submission.fullname);
    } else {
        submissionRemoved = search_duplicates(submission, settings, image, false, commentContent);
        if (submissionRemoved)
        {
            apiWrapper . submit_comment( commentContent, submission . fullname );
            apiWrapper . report_submission( submission . fullname );
        } else
            interface.insert_submission(image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string(image.get_dimensions()), submission.created, submission.isVideo, submission.title, url);
    }
}


void iterate_submissions() {
    cpr::Response submissionQuery = apiWrapper.fetch_submissions();

    json submissionList = json::parse( submissionQuery . text )[ "data" ][ "children" ];

    for ( auto submissionIter = submissionList.begin(); submissionIter != submissionList.end(); submissionIter++)
    {
        Submission submission(submissionIter.value()[ "data" ]);
        interface.switch_subreddit(submission.subreddit);
        RowResult settingsQuery = interface.get_subreddit_settings(submission.subreddit);
        SubredditSetting settings(settingsQuery);

        if (submission.isGallery) {
            for (const auto& url : submission.galleryUrls) {
                handle_image(submission, settings, url);
            }
        }
        else {
            handle_image(submission, settings, submission.url);
        }
    }
}

std::string bool_row(Row &row, int pos) {
    return row.get(pos).get<bool>() ? "true" : "false";
}

std::string int_row(Row &row, int pos) {
    return std::to_string(row.get(pos).get<int>());
}

std::string get_subreddit_settings_list(const std::string &sub) {
    auto row = interface.get_subreddit_settings(sub).fetchOne();

    char *settingsChar = new char[2000]; // 2000 is very generous

    sprintf(settingsChar, SETTINGS_STRING, bool_row(row, SS_ENABLED).c_str(), int_row(row, SS_REMOVE_THRESHOLD).c_str(), int_row(row, SS_REPORT_THRESHOLD).c_str(),
            bool_row(row, SS_ENFORCE_VIDEOS).c_str(), bool_row(row, SS_ENFORCE_IMAGES).c_str(), bool_row(row, SS_ENFORCE_LINKS).c_str(), int_row(row, SS_TIME_RANGE).c_str(),
            bool_row(row, SS_REPORT_LINKS).c_str(),bool_row(row, SS_REPORT_REPLIES).c_str(), bool_row(row, SS_ENFORCE_TITLES).c_str(), int_row(row, SS_TITLE_REMOVE_THRESHOLD).c_str(),
            int_row(row, SS_TITLE_REPORT_THRESHOLD).c_str(), int_row(row, SS_MIN_TITLE_LENGTH_TO_ENFORCE).c_str(), int_row(row, SS_REMOVE_TABLE_DUPLICATE_INT).c_str(), int_row(row, SS_OCR_TEXT_THRESHOLD).c_str());

    std::string formattedSettings(settingsChar);
    delete[] settingsChar;
    return formattedSettings;
}

void iterate_messages() {
    cpr::Response messageQuery = apiWrapper.fetch_messages();

    json messageList = json::parse( messageQuery.text )[ "data" ][ "children" ];
    for ( auto messageIter = messageList.begin(); messageIter != messageList.end(); messageIter++) {
        Message message(messageIter.value());
        apiWrapper.mark_message_as_read(message.fullname);
        if (!message.subreddit.has_value()) {
            apiWrapper.submit_comment(ERROR_NOT_FROM_SUB, message.fullname);
            continue;
        }

        if (message.subject.starts_with("invitation to moderate")) {
            std::string subreddit = get_last_word(message.subject);
            interface.add_settings_row(subreddit.substr(3, subreddit.size())); // Remove the "/r/"
            apiWrapper.accept_invite(subreddit);
            continue;
        }

        std::stringstream messageStream(message.body);
        std::string parameter; std::string value;
        std::string successes; std::string failures;
        std::string response;
        while (messageStream && messageStream.tellg() != -1) {
            messageStream >> parameter;
            if (BOOLEAN_SETTINGS.contains(parameter)) {
                messageStream >> value;
                if (value == "true" || value == "false") {
                    interface.update_subreddit_settings(message.subreddit.value(), parameter.substr(0,parameter.size()-1), value == "true" ? "1" : "0");
                    successes.append("`").append(parameter).append("`,");
                } else
                {
                    failures.append("`").append(parameter).append("`,");
                }
            }
            else if (INT_SETTINGS.contains(parameter)) {
                messageStream >> value;
                if (is_number(value)) {
                    interface.update_subreddit_settings(message.subreddit.value(), parameter.substr(0,parameter.size()-1), value);
                    successes.append("`").append(parameter).append("`,");
                } else {
                    failures.append("`").append(parameter).append("`,");
                }
            }
        }
        if (!successes.empty())
            response.append(UPDATE_SUCCESS).append(successes.substr(0, successes.size()-1)).append("\n\n");
        if (!failures.empty())
            response.append(UPDATE_FAILURE).append(failures.substr(0, failures.size()-1)).append("\n\n");
        response.append(get_subreddit_settings_list(message.subreddit.value()));
        apiWrapper.submit_comment(response, message.fullname);
    }
}

// TODO: Revisit better error handling
// TODO: Check if link is downloadable image beforehand
// TODO: Add support for links
// TODO: Add support for titles
// TODO: Create benchmark pipeline
// TODO: Add support for import
// TODO: Add support for replies report
// TODO: Add handling of ratelimit
//std::cout << submissionQuerySTR.header["X-Ratelimit-Remaining"] << std::endl;
int main()
{
    initializeToken();
    initializeTesseract();

    try {
        iterate_messages();
        iterate_submissions();
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    exit(0);

    /* <-- SUBMISSIONS --> */

    /* <-- MESSAGES --> */


    return 0;
}
