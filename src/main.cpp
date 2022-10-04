#include "api_wrapper.h"
#include "image_manipulation.h"
#include "reddit_entities.h"
#include "string_constants.h"
#include <chrono>
#include <cstdio>
#include "utils.h"
#include <list>

ApiWrapper apiWrapper;
db_interface interface( (Session("localhost", 33060, "db_user", "soleil")));
std::set<std::string> dictionnary;
std::vector<std::thread> benchmarkThreads;
tesseract::TessBaseAPI* tessBaseApi[NUMBER_THREADS];
std::binary_semaphore setSemaphore{1};

using namespace ::mysqlx;

unsigned long long get_unix_time() {
    const auto currentTime = std::chrono::system_clock::now();
    const auto currentTimeUnix = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count();
    return currentTimeUnix;
}

// Should be done only once the original token expires.
void initialize_token()
{
    apiWrapper.set_number_threads(NUMBER_THREADS);
    cpr::Response tokenQuery = apiWrapper.fetch_token();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    apiWrapper.set_token( ogToken );
    int expiresIn = jsonToken["expires_in"];
    apiWrapper.set_time_expire(get_unix_time() + expiresIn);
}

void destroy_tesseract() {
    for (auto &i : tessBaseApi)
    {
        i -> End();
        delete i;
    }
}

void initialize_tesseract()
{
    for (auto &i : tessBaseApi) {
        i = new tesseract::TessBaseAPI();
        // Initialize tesseract-ocr with English, without specifying tessdata path
        if (i->Init( nullptr, "eng")) {
            fprintf(stderr, "Could not initialize tesseract.\n");
            exit(1);
        }

        // Or PSM_AUTO_OSD for extra info (OSD). Difference not significant.
        // See : https://pyimagesearch.com/2021/11/15/tesseract-page-segmentation-modes-psms-explained-how-to-improve-your-ocr-accuracy/
        i->SetVariable("debug_file", "/dev/null");
        i->SetPageSegMode( tesseract::PSM_AUTO);   //matches -psm 1 from the command line
        atexit(( destroy_tesseract ));
    }
}


// NOTE : Filtering non-words seemed interesting at first, but it greatly reduced the amount of detected reposts (more than 50%).
// May also reduce false positives but I doubt it since you're essentially losing data. This function will remain unused for the moment.
void initialize_dictionnary() {
    std::string word;

    std::ifstream usDict("/usr/share/dict/american-english");
    while (getline (usDict, word)) {
        if (word.size() > 1)
            dictionnary.insert(word);
    }
    usDict.close();

    std::ifstream ukDict("/usr/share/dict/british-english");
    while (getline (ukDict, word)) {
        if (word.size() > 1)
            dictionnary.insert(word);
    }
    ukDict.close();
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

std::string create_image_markdown_header( const std::string &author, const std::string &date, const std::string &dimensions) {
    return "**OP:** " + author + "\n\n**Date:** " + date + "\n\n**Dimensions:** " + dimensions + "\n\n**Duplicates:**\n\n" + MARKDOWN_TABLE_IMAGE;
}

std::string create_markdown_header( const std::string &author, const std::string &date, const std::string &MARKDOWN_HEADER) {
    return "**OP:** " + author + "\n\n**Date:** " + date + "\n\n**Duplicates:**\n\n" + MARKDOWN_HEADER;
}

std::string create_image_markdown_row( int number, int similarity, Row &row, long long submissionTime) {
    auto id = row.get(DB_ID).get<std::string>();
    auto author = row.get(DB_AUTHOR).get<std::string>();
    auto dimensions = row.get(DB_DIMENSIONS).get<std::string>();

    auto unixDate = row.get(DB_DATE).get<time_t>();

    auto title = row.get(DB_TITLE).get<std::string>();
    auto url = row.get(DB_URL).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + unix_time_to_string(unixDate)
           + " | " + get_time_interval(unixDate, submissionTime) + " | " + "[" + std::to_string(similarity) + "%](" + url + ") | "
           + dimensions + " | [" + title + "](https://redd.it/" + id + ")\n";
}


std::string create_link_markdown_row( int number, Row &row, long long submissionTime) {
    auto author = row.get(DB_AUTHOR).get<std::string>();
    auto id = row.get(DB_ID).get<std::string>();
    auto unixDate = row.get(DB_DATE).get<time_t>();
    auto title = row.get(DB_TITLE).get<std::string>();
    auto url = row.get(DB_URL).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + unix_time_to_string(unixDate)
           + " | " + get_time_interval(unixDate, submissionTime) + " | [url](https://" + url + ") | [" + title + "](https://redd.it/" + id + ")\n";
}

std::string create_title_markdown_row( int number, Row &row, long long submissionTime, int similarity) {
    auto author = row.get(DB_AUTHOR).get<std::string>();
    auto id = row.get(DB_ID).get<std::string>();
    auto unixDate = row.get(DB_DATE).get<time_t>();
    auto title = row.get(DB_TITLE).get<std::string>();
    auto url = row.get(DB_URL).get<std::string>();
    return std::to_string(number) + " | " + "[/u/" + author + "](https://www.reddit.com/user/" + author + ") | " + unix_time_to_string(unixDate)
           + " | " + get_time_interval(unixDate, submissionTime) + "| [" +
           std::to_string(similarity) + "%](https://" + url + ") | [" + title + "](https://redd.it/" + id + ")\n";
}

std::string get_comment_fullname(const cpr::Response &query) {
    std::string fullname = "t1_";
    json parsedQuery = json::parse(query.text);
    fullname.append(parsedQuery["json"]["data"]["things"][0]["data"]["id"]);
    return fullname;
}

std::string submit_comment(const std::string &content, const std::string &fullname) {
    auto query = apiWrapper.submit_comment(content, fullname);
    std::string commentFullname = get_comment_fullname(query);
    apiWrapper.distinguish_comment(commentFullname);
    return commentFullname;
}

bool determine_remove(int imageSimilarity, int imageThreshold, double textSimilarity, int textLength1, int textLength2) {
    return (textLength1 > 35 && textLength2 > 35 && textSimilarity > 75)
    || (textLength1 > 5 && textLength2 > 5 && textSimilarity > 65 && imageSimilarity > 50)
    || (textSimilarity > 60 && imageSimilarity >= imageThreshold)
    || (textLength1 < 5 && textLength2 < 5 && imageSimilarity >= imageThreshold);
}

// this is a test with about 35 chars !
bool determine_report(int imageSimilarity, int imageThreshold, double textSimilarity, int textLength1, int textLength2) {
    return (textLength1 > 5 && textLength2 > 5 && textSimilarity > 65 && imageSimilarity > 75)
           || (textSimilarity > 60 && imageSimilarity >= imageThreshold)
           || (textLength1 < 5 && textLength2 < 5 && imageSimilarity >= imageThreshold);
}

std::string get_modmail_footer(const std::string &sub, const std::string &context) {
    std::string modmailContent;
    modmailContent.append(MODMAIL_FOOTER_1).append(sub).append(MODMAIL_FOOTER_2).append(context).append(MODMAIL_FOOTER_3);
    return modmailContent;
}

void post_action_comment( std::string commentContent, const std::vector<std::string> &removeComment, const std::vector<std::string> &reportComment,
                          const Submission &submission, const std::string &HEADER, const bool reportReplies) {
    if (!removeComment.empty()) {
        commentContent = HEADER + commentContent;
        for (const auto &str : removeComment)
            commentContent.append(str);
        if (reportReplies)
            commentContent.append( FOOTER_REMOVE );
        else
            commentContent.append( get_modmail_footer(submission.subreddit, submission.shortlink) );
        apiWrapper.remove_submission(submission.fullname);
        submit_comment(commentContent, submission.fullname);
    } else if (!reportComment.empty()) {
        for (const auto &str : reportComment)
            commentContent.append(str);
        apiWrapper.report_submission(submission.fullname, DUPLICATE_FOUND_MSG);
        std::string commentFullname = submit_comment(commentContent, submission.fullname);
        apiWrapper.remove_submission(commentFullname);
    }
}

bool search_image_duplicates( const Submission &submission, const SubredditSetting &settings, Image &image) {
    std::string commentContent;
    mpz_class hash; mpz_class mpzhash;
    int numberDuplicates = 0;
    std::vector<std::string> reportComment; std::vector<std::string> removeComment;

    auto query = interface.get_image_rows(submission.subreddit);

    commentContent.append(create_image_markdown_header( submission . author, unix_time_to_string( submission . created ), dimensions_to_string( image . get_dimensions())));
    std::string imageOcrString = image.get_text();//image.filter_non_words(dictionnary);

    for (auto row : *query) {
        if (numberDuplicates > settings.removal_table_duplicate_number)
            break;

        if (get_days_interval(row.get(DB_DATE).get<u_long>(), submission.created) > settings.time_range)
            continue;

        std::string ocrSTR = row.get(DB_OCRSTRING).get<std::string>();

        int strSimilarity = (int)get_string_similarity(ocrSTR, imageOcrString);

        std::string strhash = row.get(DB_10PXHASH).get<std::string>(); // get 10px hash or 8px one
        mpzhash.set_str(strhash, 10);
        int similarity = image.compareHash10x10( mpzhash );

        //std::cout << submission.title << " vs " << row.get(DB_TITLE).get<std::string>() << " text (" << strSimilarity << " @ " << imageOcrString.size() << "&" << ocrSTR.size() << " ) (" << (similarity) << ")";

        if (determine_remove(similarity, settings.remove_threshold, strSimilarity, imageOcrString.size(), ocrSTR.size())) {
            //std::cout << " (REMOVED)";
            numberDuplicates++;
            removeComment.push_back( create_image_markdown_row( numberDuplicates, similarity, row, submission . created ));
        } else if (removeComment.empty()) { // If the submission doesn't fit the criterias for removal, check criterias for report
            strhash = row.get(DB_8PXHASH).get<std::string>();
            mpzhash.set_str(strhash, 10);
            similarity = image.compareHash8x8( mpzhash );

            if (determine_report(similarity, settings.report_threshold, strSimilarity, imageOcrString.size(), ocrSTR.size())) {
                //std::cout << " (REPORTED)";
                numberDuplicates++;
                reportComment.push_back(create_image_markdown_row( numberDuplicates, similarity, row, submission . created ));
            }
        }
        //std::cout << std::endl;
    }

    if (numberDuplicates > 0) {
        post_action_comment(commentContent, removeComment, reportComment, submission, HEADER_REMOVE_IMAGE, settings.report_replies);
    }

    return numberDuplicates > 0;
}

std::string get_last_word(std::string &s) {
    auto index = s.find_last_of(' ');
    std::string last_word = s.substr(++index);
    return last_word;
}

void process_image(Image &image, const std::string &url, int threadNumber) {
    apiWrapper.download_image(url, threadNumber);
    image.matrix = imread(IMAGE_NAME);
    image . extract_text(threadNumber);

    image.computeHash8x8();
    image . computeHash10x10();
}


bool handle_image(const Submission &submission, const SubredditSetting &settings, Image &image, const std::string &url, int threadNumber) {
    std::string commentContent;
    process_image(image, url, threadNumber);
    bool submissionRemoved = search_image_duplicates( submission, settings, image );

    return submissionRemoved;
}

bool handle_link(const Submission &submission, const SubredditSetting &settings) {
    std::string commentContent;
    auto query = interface.get_link_rows(submission.subreddit);

    commentContent.append( create_markdown_header( submission . author, unix_time_to_string( submission . created ), MARKDOWN_TABLE_LINK));

    int numberDuplicates = 0;

    for (auto row : *query) {
        if (numberDuplicates > settings.removal_table_duplicate_number)
            break;

        if (get_days_interval(row.get(DB_DATE).get<u_long>(), submission.created) > settings.time_range)
            continue;

        if (submission.url == row.get(DB_URL).get<std::string>()) {
            numberDuplicates++;
            commentContent.append( create_link_markdown_row( numberDuplicates, row, submission . created ));
        }
    }

    if (numberDuplicates > 0) {
        if (settings.report_links) {
            commentContent.append(FOOTER_REMOVE);
            apiWrapper.report_submission(submission.fullname, DUPLICATE_FOUND_MSG);
            std::string commentFullname = submit_comment(commentContent, submission.fullname);
            apiWrapper.remove_submission(commentFullname);
        }
        else
        {
            commentContent = HEADER_REMOVE_LINK + commentContent;
            if (settings.report_replies)
                commentContent.append( FOOTER_REMOVE );
            else
                commentContent.append( get_modmail_footer(submission.subreddit, submission.shortlink) );
            apiWrapper . remove_submission( submission . fullname );
            submit_comment(commentContent, submission.fullname);
        }
    }
    return numberDuplicates > 0;
}

bool handle_title(const Submission &submission, const SubredditSetting &settings) {
    std::string commentContent;
    std::vector<std::string> reportComment; std::vector<std::string> removeComment;

    auto query = interface.get_title_rows(submission.subreddit, settings.min_title_length_to_enforce);

    commentContent.append( create_markdown_header( submission . author, unix_time_to_string( submission . created ), MARKDOWN_TABLE_TITLE));

    int numberDuplicates = 0;

    for (auto row : *query)
    {
        if ( numberDuplicates > settings . removal_table_duplicate_number )
            break;

        if ( get_days_interval( row . get( DB_DATE ) . get<u_long>(), submission . created ) > settings . time_range )
            continue;

        int similarity = (int)get_string_similarity(submission.title, row.get(DB_TITLE).get<std::string>());

        if (similarity > settings.title_remove_threshold) {
            removeComment.push_back(create_title_markdown_row( numberDuplicates, row, submission . created, similarity ));
            numberDuplicates++;
        }
        else if (similarity > settings.title_report_threshold) {
            reportComment.push_back(create_title_markdown_row( numberDuplicates, row, submission . created, similarity ));
            numberDuplicates++;
        }
    }
    if (numberDuplicates > 0) {
        post_action_comment(commentContent, removeComment, reportComment, submission, HEADER_REMOVE_TITLE, settings.report_replies);
    }
    return numberDuplicates > 0;
}

void import_submissions(const std::string &subreddit, int threadNumber=0) {
    for (const std::string &range : {"month", "year", "all"}) {
        cpr::Response submissionQuery = apiWrapper.fetch_top_submissions(subreddit, range);

        json submissionList = json::parse( submissionQuery . text )[ "data" ][ "children" ];

        for (auto &submissionIter : submissionList) {
            try {
                Submission submission(submissionIter[ "data" ]);
                if (interface.is_submission_saved(submission.subreddit, submission.id))
                    continue;
                else
                    interface.save_submission(submission.subreddit, submission.id);

                if (submission.type == IMAGE || submission.type == VIDEO) {
                    Image image; // TODO: Code repeated here... figure out a way to plug-in a function
                    if (submission.isGallery) {
                        for (const std::string &url : submission.galleryUrls)
                        {
                            process_image(image, url, threadNumber);
                            interface.insert_submission(submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string(image.get_dimensions()), submission.created, submission.type == LINK, submission.title, url);
                        }
                    } else {
                        process_image(image, submission.url, threadNumber);
                        interface.insert_submission(submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string(image.get_dimensions()), submission.created, submission.type == LINK, submission.title, submission.url);
                    }
                }
                else {
                    interface.insert_submission(submission.subreddit, "", "", "", submission.id, submission.author, "", submission.created, submission.type == LINK, submission.title, submission.url);
                }
            } catch (std::exception &e) {
                std::cerr << "IMPORT SUBMISSIONS:" << e.what() << std::endl;
            }
        }
    }
}

void insert_submission(const Submission &submission, Image &image, bool isMedia) {
    if (isMedia) {
        if ( submission . isGallery )
        {
            for ( const auto &url : submission . galleryUrls )
                interface.insert_submission(submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string(image.get_dimensions()), submission.created, submission.type == LINK, submission.title, url);
        } else
            interface.insert_submission(submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string(image.get_dimensions()), submission.created, submission.type == LINK, submission.title, submission.url);
    }
    else // link or selftext
        interface.insert_submission(submission.subreddit, "", "", "", submission.id, submission.author, "", submission.created, submission.type == LINK, submission.title, submission.url);
}

void process_submission(bool *finished, std::set<std::string> *sub2thread, std::set<std::string> *requiredOrder, const Submission &submission, const SubredditSetting &settings, int threadNumber) {
    try {
        bool submissionRemoved = false;

        bool isMedia = ( submission . type == IMAGE && settings . enforce_images ) ||
                       ( submission . type == VIDEO && settings . enforce_videos );
        Image image;
        // 1 - Submission is a media
        if (isMedia)
        {
            if ( submission . isGallery )
            {
                for ( const auto &url : submission . galleryUrls )
                    submissionRemoved = handle_image( submission, settings, image, url, threadNumber );
            } else
                submissionRemoved = handle_image( submission, settings, image, submission . url, threadNumber);
        } // 2 - submission is a link
        else if ( settings . enforce_links && submission.type == LINK )
            submissionRemoved = handle_link( submission, settings );

        // 3 - Submission is either a media or a link, but no duplicates have been found   ;  search for titles
        if ( !submissionRemoved && settings . enforce_titles && submission . title . size() >= settings . min_title_length_to_enforce )
        {
            submissionRemoved = handle_title( submission, settings );
        }

        if (!submissionRemoved)
            insert_submission(submission, image, isMedia);

    } catch (std::exception &e) {
        std::cerr << "EXCEPTION ON THREAD-SHARING:" << e.what() << std::endl;
    }
    // Remove blockers
    setSemaphore.acquire();
    sub2thread->erase(submission.subreddit);
    requiredOrder->insert(submission.subreddit); // sure, we've freed the sub, but not until the loop has started over
    setSemaphore.release();

    *finished = true;
}

// Choosing the next thread in which to process a submission of the subreddit corresponding to 'sub'
[[nodiscard]] int choose_thread(const std::string &sub, const bool finished[NUMBER_THREADS], std::set<std::string> &sub2thread, std::set<std::string> &requiredOrder) {
    // If a thread with the same sub is already running, then wait. If a sub has finished in the current iteration, then wait as well.
    setSemaphore.acquire();
    if (sub2thread.contains(sub) || requiredOrder.contains(sub))
    {
        setSemaphore.release();
        return -1;
    }
    setSemaphore.release();

    while (true) {
        // Loop until a thread has finished
        for (int i = 0; i < NUMBER_THREADS; i++) {
            if (finished[i])
            {
                return i;
            }
        }
    }
}

inline void next_iteration(_List_iterator<Submission> &it, std::list<Submission> &list) {
    auto copy = it;
    it++;
    list.erase(copy);
}

void populate_list(std::list<Submission> &submissionList, json &submissionJSON) {
    // Inversed loop - we want to check older submissions first
    for ( auto submissionIter = submissionJSON.rbegin(); submissionIter != submissionJSON . rend(); submissionIter++ )
    {
        try
        {
            Submission submission( submissionIter . value()[ "data" ] );
            if ( !interface . is_submission_saved( submission . subreddit, submission . id ))
            {
                submissionList.push_back(submission);
                interface . save_submission( submission . subreddit, submission . id );
            }

        } catch ( std::exception &e )
        {
            std::cerr << "EXCEPTION ON LIST-BUILDING:" << e.what() << std::endl;
        }
    }
}


void iterate_submissions() {
    cpr::Response submissionQuery = apiWrapper . fetch_submissions();

    json submissionJSON = json::parse( submissionQuery . text )[ "data" ][ "children" ];

    std::list<Submission> submissionList;

    populate_list(submissionList, submissionJSON);

    benchmarkThreads.reserve(NUMBER_THREADS);

    bool finished[NUMBER_THREADS]; for (int i = 0; i < NUMBER_THREADS; i++) finished[i] = true;
    bool alreadyCalled[NUMBER_THREADS]; for (int i = 0; i < NUMBER_THREADS; i++) alreadyCalled[i] = false;

    std::set<std::string> sub2thread; // Indicates if a given subreddit has a submission being treated in a thread

    // If a subreddit is present, indicates that it has been freed by a thread, but we have to wait for the loop to start over
    // before sending a new submission of the same subreddit in a thread.
    // The reason behind is that we want submissions to be treated in order (from the oldest submission to the newest one).
    std::set<std::string> requiredOrder;

    while (!submissionList.empty()) {

        setSemaphore.acquire();
        requiredOrder.clear();
        setSemaphore.release();

        for (auto it = submissionList.begin(); it != submissionList.end(); )
        {
            auto &submission = *it;

            RowResult settingsQuery = interface . get_subreddit_settings( submission . subreddit );
            SubredditSetting settings( settingsQuery );
            if ( !settings . enabled )
            {
                next_iteration(it, submissionList);
                continue;
            }

            int thread = choose_thread(submission.subreddit, finished, sub2thread, requiredOrder);
            if (thread != -1) {
                // Starting a new thread, mark it as not finished & limit subreddit to avoid race-conditions
                finished[thread] = false;

                // Threads can interact with the set too, hence the need for a semaphore
                setSemaphore.acquire();
                sub2thread.insert(submission.subreddit); // the subreddit is busy
                setSemaphore.release();

                // Calling join() on a never-initialized thread will cause exceptions, hence this IF
                if (!alreadyCalled[thread])
                    alreadyCalled[thread] = true;
                else {
                    benchmarkThreads[thread].join();
                }
                benchmarkThreads[thread] = std::thread(process_submission, finished + thread, &sub2thread, &requiredOrder, submission, settings, thread);
                next_iteration(it, submissionList);
            }
            else
                it++;
        }
    }
    for (int i = 0; i < NUMBER_THREADS; i++) {
        if (alreadyCalled[i])
            benchmarkThreads[i].join();
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

// Notice : reddit's API was returning false for a small test subreddit, even though it existed... seems to work for every other sub but watch out for bug
bool subreddit_exists(const std::string &sub) {
    if (sub.size() > 50 || sub.size() < 2)
        return false;
    auto query = apiWrapper.subreddit_exists(sub);
    auto json = json::parse(query.text);
    return !json["subreddits"].empty();
}

void parse_subreddit_settings(const Message &message) {
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
                successes.append("`").append(parameter).append("`,"); // Create & format list of successful boolean parameters
            } else
            {
                failures.append("`").append(parameter).append("`,");
            }
        }

        else if (INT_SETTINGS.contains(parameter)) {
            messageStream >> value;
            if (is_number(value)) {
                interface.update_subreddit_settings(message.subreddit.value(), parameter.substr(0,parameter.size()-1), value);
                successes.append("`").append(parameter).append("`,"); // Create & format list of successful int parameters
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

void iterate_messages() {
    cpr::Response messageQuery = apiWrapper.fetch_messages();

    json messageList = json::parse( messageQuery.text )[ "data" ][ "children" ];
    for ( auto messageIter = messageList.begin(); messageIter != messageList.end(); messageIter++) {
        Message message(messageIter.value());
        apiWrapper.mark_message_as_read(message.fullname);

        if (message.isReply) {
            RowResult settingsQuery = interface.get_subreddit_settings(message.subreddit.value());
            SubredditSetting settings(settingsQuery);
            if (settings.report_replies)
                apiWrapper.report_submission(message.fullname, FALSE_POSITIVE_MSG);
            else {
                std::string modmailContent;
                modmailContent.append(MODMAIL_FOOTER_1).append(message.subreddit.value()).append(MODMAIL_FOOTER_2).append(message.context.value()).append(MODMAIL_FOOTER_3);
                submit_comment(modmailContent, message.fullname);
            }
            continue;
        }

        if (!message.subreddit.has_value()) {
            apiWrapper.submit_comment(ERROR_NOT_FROM_SUB, message.fullname);
            continue;
        }

        if (message.subject.starts_with("invitation to moderate")) {
            std::string subreddit = get_last_word(message.subject);
            std::string subredditWithoutR = subreddit.substr(3, subreddit.size()); // Remove the /r/
            if (!subreddit_exists(subredditWithoutR)) // This methods acts against SQL injection
                continue;
            if (!interface.settings_exist(subredditWithoutR))
                interface.add_settings_row(subredditWithoutR);
            if (!interface.subreddit_table_exists(subredditWithoutR))
                interface . create_table_duplicates( subredditWithoutR );
            if (!interface.subreddit_table_exists(subredditWithoutR + "_saved"))
                interface . create_table_saved( subredditWithoutR );
            apiWrapper.accept_invite(subreddit);
            import_submissions(subredditWithoutR);
            std::string content;
            content.append(THANKS_INVITE_1).append(subredditWithoutR).append(THANKS_INVITE_2).append("\n\n").append(get_subreddit_settings_list(subredditWithoutR));
            continue;
        }

        // If the message is a private DM, comes from a subreddit, & is not a mod invite, then it's probably a
        // settings DM. => Try to find & parse settings
        parse_subreddit_settings(message);
    }
}

// Adjust sleep in function of new submissions. Don't waste our API requests when there are few submissions
unsigned calculate_sleep() {
    auto substract = (unsigned)(interface.get_new_submissions() * (0.3));
    if (substract > 30)
        substract = 30;
    unsigned sleep = 30 - substract;
    return sleep;
}

/*
 * Removal 45/45
 * Reports 9/10
 * */
// TODO : Implement multithreading with sleeps on threads when needed + mutex on DB for each sub locked between a select and an insert

int main()
{
    int count = 0;
    initialize_tesseract();

    while (true) {
        try {
            if (apiWrapper.get_time_expire() - get_unix_time() < 10000 || apiWrapper.get_time_expire() == 0)
                initialize_token();

            iterate_messages();
            iterate_submissions();
        } catch (std::exception &e) {
            std::cerr << "EXCEPTION : " << e.what() << std::endl;
        }
        std::cout << count++ << std::endl;
        sleep(calculate_sleep());
    }
}
