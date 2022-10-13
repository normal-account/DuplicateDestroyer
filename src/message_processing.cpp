#include "message_processing.h"

bool is_number(const std::string& s) {
    int count = 0;
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) {
        count++; // counting the amount of digits ; we don't want a number above 999 999 999
        ++it;
    }
    return !s.empty() && it == s.end() && count <= 9;
}

std::string get_last_word(std::string &s) {
    auto index = s.find_last_of(' ');
    std::string last_word = s.substr(++index);
    return last_word;
}

std::string bool_row(Row &row, int pos) {
    return row.get(pos).get<bool>() ? "true" : "false";
}

std::string int_row(Row &row, int pos) {
    return std::to_string(row.get(pos).get<int>());
}

std::string get_subreddit_settings_list(const std::string &sub, int threadNumber) {
    auto row = interfaces[threadNumber]->get_subreddit_settings( sub).fetchOne();

    char *settingsChar = new char[2000]; // 2000 is a very generous magic number

    sprintf(settingsChar, SETTINGS_STRING, bool_row(row, SS_ENABLED).c_str(), int_row(row, SS_REMOVE_THRESHOLD).c_str(), int_row(row, SS_REPORT_THRESHOLD).c_str(),
            bool_row(row, SS_ENFORCE_VIDEOS).c_str(), bool_row(row, SS_ENFORCE_IMAGES).c_str(), bool_row(row, SS_ENFORCE_LINKS).c_str(), int_row(row, SS_TIME_RANGE).c_str(),
            bool_row(row, SS_REPORT_LINKS).c_str(),bool_row(row, SS_REPORT_REPLIES).c_str(), bool_row(row, SS_ENFORCE_TITLES).c_str(), int_row(row, SS_TITLE_REMOVE_THRESHOLD).c_str(),
            int_row(row, SS_TITLE_REPORT_THRESHOLD).c_str(), int_row(row, SS_MIN_TITLE_LENGTH_TO_ENFORCE).c_str(), int_row(row, SS_REMOVE_TABLE_DUPLICATE_INT).c_str(), int_row(row, SS_OCR_TEXT_THRESHOLD).c_str());

    std::string formattedSettings(settingsChar);
    delete[] settingsChar;
    return formattedSettings;
}

// Warning : reddit's API may return false for very small subreddits (even though they exist)
bool subreddit_exists(const std::string &sub) {
    if (sub.size() > 50 || sub.size() < 2)
        return false;
    auto query = apiWrapper.subreddit_exists(sub);
    auto json = json::parse(query.text);
    return !json["subreddits"].empty();
}

void parse_subreddit_settings(const Message &message, int threadNumber) {
    std::stringstream messageStream(message.body);
    std::string parameter; std::string value;
    std::string successes; std::string failures;
    std::string response;
    while (messageStream && messageStream.tellg() != -1) {
        messageStream >> parameter;
        if (BOOLEAN_SETTINGS.contains(parameter)) {
            messageStream >> value;
            if (value == "true" || value == "false") {
                interfaces[threadNumber]->update_subreddit_settings( message.subreddit.value(), parameter.substr( 0, parameter.size() - 1), value == "true" ? "1" : "0");
                successes.append("`").append(parameter).append("`,"); // Create & format list of successful boolean parameters
            } else
            {
                failures.append("`").append(parameter).append("`,");
            }
        }

        else if (INT_SETTINGS.contains(parameter)) {
            messageStream >> value;
            if (is_number(value)) {
                interfaces[threadNumber]->update_subreddit_settings( message.subreddit.value(), parameter.substr( 0, parameter.size() - 1), value);
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
    // Only submit comment if settings have been parsed
    if (!failures.empty() || !successes.empty()) {
        response.append(get_subreddit_settings_list(message.subreddit.value(), threadNumber));
        apiWrapper.submit_comment(response, message.fullname);
    }
}

void iterate_messages(int threadNumber) {
    cpr::Response messageQuery = apiWrapper.fetch_messages();

    json messageList = json::parse( messageQuery.text )[ "data" ][ "children" ];
    for ( auto messageIter = messageList.begin(); messageIter != messageList.end(); messageIter++) {
        Message message(messageIter.value());
        apiWrapper.mark_message_as_read(message.fullname);

        if (message.isReply) {
            RowResult settingsQuery = interfaces[threadNumber]->get_subreddit_settings( message.subreddit.value());
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
            if (!interfaces[threadNumber]->settings_exist( subredditWithoutR))
                interfaces[threadNumber]->add_settings_row( subredditWithoutR);

            if (!interfaces[threadNumber]->subreddit_table_exists( subredditWithoutR))
                interfaces[threadNumber]->create_table_duplicates( subredditWithoutR );

            if (!interfaces[threadNumber]->subreddit_table_exists( subredditWithoutR + "_saved"))
                interfaces[threadNumber]->create_table_saved( subredditWithoutR );

            apiWrapper.accept_invite(subreddit);
            std::string content;
            content.append(THANKS_INVITE_1).append(subredditWithoutR).append(THANKS_INVITE_2).append("\n\n").append(get_subreddit_settings_list(subredditWithoutR, threadNumber));
            apiWrapper.submit_comment(content, message.id);
            import_submissions(subredditWithoutR, threadNumber);
            continue;
        }

        // If the message is a private DM, comes from a subreddit, & is not a mod invite, then it's probably a
        // settings DM. => Try to find & parse settings
        parse_subreddit_settings(message, threadNumber);
    }
}