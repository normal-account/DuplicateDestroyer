#define DB_OCRSTRING 0
#define DB_10PXHASH 1
#define DB_8PXHASH 2
#define DB_ID 3
#define DB_AUTHOR 4
#define DB_DIMENSIONS 5
#define DB_DATE 6
#define DB_IS_VIDEO 7
#define DB_TITLE 8
#define DB_URL 9

#define SS_SUBREDDIT 0
#define SS_ENABLED 1
#define SS_REMOVE_THRESHOLD 3
#define SS_REPORT_THRESHOLD 4
#define SS_ENFORCE_VIDEOS 5
#define SS_ENFORCE_IMAGES 6
#define SS_ENFORCE_LINKS 7
#define SS_TIME_RANGE 8
#define SS_REPORT_LINKS 9
#define SS_REPORT_REPLIES 10
#define SS_ENFORCE_TITLES 11
#define SS_TITLE_REMOVE_THRESHOLD 12
#define SS_TITLE_REPORT_THRESHOLD 13
#define SS_MIN_TITLE_LENGTH_TO_ENFORCE 14
#define SS_REMOVE_TABLE_DUPLICATE_INT 15
#define SS_OCR_TEXT_THRESHOLD 16

const std::string HEADER_REMOVE_IMAGE = "**Your submission has been removed because it has been posted on the subreddit recently.**\n\n";

const std::string HEADER_REMOVE_LINK = "**Your submission has been removed because at least 1 submission with the same URL has posted on the subreddit recently.**";

const std::string HEADER_REMOVE_TITLE = "**Your submission has been removed because at least 1 submission with the same title has posted on the subreddit recently.**";

const std::string FOOTER_REMOVE = "\n\nI am a bot. If you believe this was sent in error, reply to this comment and a"
                                        " moderator will review your post. **Do not delete your post or moderators won't be able to review it.**";

/*const std::string FOOTER_REMOVE_IMAGE_2 = "I am a bot. If you believe this was sent in error, [please message the subreddit moderators here]"
                                          "(https://www.reddit.com/message/compose?to=%2Fr%2F\" + settings[0] +  \"&subject=My+post+has+been+wrongfully\"\n"
                                          "            \"+removed&message=My%20post%20has%20been%20wrongfully%20removed%20by%20your%20repost%20bot,%20/u/\"\n"
                                          "            \"DuplicateDestroyer.%20Here%20is%20a%20link%20to%20my%20post:%20\" + submission.shortlink + \").\"\n"
                                          "            \" **Do not delete your post or moderators won't be able to review it.**"*/

const std::string MARKDOWN_TABLE_IMAGE = "N | User | Date | Posted... | Similarity | Dimensions | Title\n:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:\n";

const std::string MARKDOWN_TABLE_LINK = "N | User | Date | Posted... | URL | Title\n:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:\n";

const std::string MARKDOWN_TABLE_TITLE = "N | User | Date | Posted... | Similarity | Title\n:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:\n";

const std::string ERROR_SETTINGS_MSG = "**ERROR** : You've entered an invalid value for one of the settings. Please fix the issue and try again.";

const std::string ERROR_NOT_FROM_SUB = "**ERROR** : Setting messages need to be sent directly from the designated subreddit.";

const std::string UPDATE_SUCCESS_MSG = "Subreddit settings have been successfully updated.";

const std::string MODMAIL_FOOTER_1 = "I am a bot. If you believe this was sent in error, [please message the subreddit moderators here]"
                                   "(https://www.reddit.com/message/compose?to=%2Fr%2F";
// Subreddit between
const std::string MODMAIL_FOOTER_2 = "&subject=My+post+has+been+wrongfully+removed&message=My%20post%20has%20been%20wrongfully%20removed%20by%20your%20repost%20bot,%20/u/"
                                   "DuplicateDestroyer.%20Here%20is%20a%20link%20to%20my%20post:%20https%3A%2F%2Fredd.it%2F";
// Submission shortlink between
const std::string MODMAIL_FOOTER_3 = "). **Do not delete your post or moderators won't be able to review it.**";

const char *const SETTINGS_STRING = "Current subreddit settings :\n\n    enabled : %s\n    remove_threshold: %s\n    report_threshold: %s\n    enforce_videos: %s\n"
                                    "    enforce_images: %s\n    enforce_links: %s\n    time_range: %s\n    report_links: %s\n    report_replies: %s\n"
                                    "    enforce_titles: %s\n    title_remove_threshold: %s\n    title_report_threshold: %s\n    min_title_length_to_enforce: %s\n"
                                    "    removal_table_duplicate_number: %s\n    ocr_text_threshold: %s";

const std::string THANKS_INVITE_1 = "Thank you for inviting me on /r/";
// Subreddit name between these 2
const std::string THANKS_INVITE_2 = ". I will save the top posts of your subreddit in my "
                                    "database before scanning your /new section, which could take some "
                                    "time. Feel free to message /r/DuplicateDestroyer if you have questions"
                                    " or concerns.\n\n";

const std::string UPDATE_SUCCESS = "Successfully updated the following parameters : ";

const std::string UPDATE_FAILURE = "Failure to update the following parameters due to invalid values : ";

const std::set<std::string> BOOLEAN_SETTINGS = {
        "enabled:",
        "enforce_images:",
        "enforce_videos:",
        "enforce_links:",
        "enforce_titles:",
        "report_replies:",
        "report_links:"
};

const std::set<std::string> INT_SETTINGS = {
        "remove_threshold:",
        "report_threshold:",
        "title_remove_threshold:",
        "title_report_threshold:",
        "min_title_length_to_enforce:",
        "time_range:",
        "removal_table_duplicate_number:",
        "ocr_text_threshold:"
};
