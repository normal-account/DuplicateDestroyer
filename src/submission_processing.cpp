#include "submission_processing.h"

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

// Returns true if the described submission fits the criteria for removal
bool determine_remove(int imageSimilarity, int imageThreshold, double textSimilarity, int textLength1, int textLength2) {
    return (textLength1 > 5 && textLength2 > 5 && textSimilarity > 70 && imageSimilarity > 65)
           || (textSimilarity > 60 && imageSimilarity >= imageThreshold)
           || (textLength1 < 5 && textLength2 < 5 && imageSimilarity >= imageThreshold);
}

// Returns true if the described submission fits the criteria for reporting
bool determine_report(int imageSimilarity, int imageThreshold, double textSimilarity, int textLength1, int textLength2) {
    return (textLength1 > 35 && textLength2 > 35 && textSimilarity > 75)
           || (textLength1 > 5 && textLength2 > 5 && textSimilarity > 65 && imageSimilarity > 75)
           || (textSimilarity > 60 && imageSimilarity >= imageThreshold)
           || (textLength1 < 5 && textLength2 < 5 && imageSimilarity >= imageThreshold);
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

bool search_image_duplicates( const Submission &submission, const SubredditSetting &settings, Image &image, int threadNumber) {
    std::string commentContent;
    mpz_class hash; mpz_class mpzhash;
    int numberDuplicates = 0;
    std::vector<std::string> reportComment; std::vector<std::string> removeComment;

    auto query = interfaces[threadNumber]->get_image_rows( submission.subreddit);

    commentContent.append(create_image_markdown_header( submission . author, unix_time_to_string( submission . created ), dimensions_to_string( image . get_dimensions())));
    std::string imageOcrString = image.get_text();//image.filter_non_words(dictionnary);

    for (auto row = query->begin(); row != query->end(); ++row) {
        if (numberDuplicates > settings.removal_table_duplicate_number)
            break;

        if (get_days_interval((*row).get(DB_DATE).get<u_long>(), submission.created) > settings.time_range)
            continue;

        if ((*row).get(DB_8PXHASH).get<std::string>() == "3069585302236515101")
            continue;

        std::string ocrSTR = (*row).get(DB_OCRSTRING).get<std::string>();

        int strSimilarity = (int)get_string_similarity(ocrSTR, imageOcrString);
        int tableStrSimilarity = imageOcrString.size() > 5 ? strSimilarity : -1; // for the create_image_markdown_row function

        std::string strhash = (*row).get(DB_10PXHASH).get<std::string>(); // get 10px hash
        mpzhash.set_str(strhash, 10);
        int similarity = image.compareHash10x10( mpzhash );


        if (determine_remove(similarity, settings.remove_threshold, strSimilarity, imageOcrString.size(), ocrSTR.size())) {
            if (ApiWrapper::image_deleted((*row).get(DB_URL).get<std::string>()))
                continue;
            numberDuplicates++;
            removeComment.push_back( create_image_markdown_row( numberDuplicates, similarity, *row, submission . created , tableStrSimilarity));
        } else if (removeComment.empty()) { // If the submission doesn't fit the criterias for removal, check criterias for reports
            strhash = (*row).get(DB_8PXHASH).get<std::string>();
            mpzhash.set_str(strhash, 10);
            similarity = image.compareHash8x8( mpzhash );

            if (determine_report(similarity, settings.report_threshold, strSimilarity, imageOcrString.size(), ocrSTR.size())) {
                if (ApiWrapper::image_deleted((*row).get(DB_URL).get<std::string>()))
                    continue;
                numberDuplicates++;
                reportComment.push_back(create_image_markdown_row( numberDuplicates, similarity, *row, submission . created, tableStrSimilarity));
            }
        }
    }

    if (numberDuplicates > 0) {
        post_action_comment(commentContent, removeComment, reportComment, submission, HEADER_REMOVE_IMAGE, settings.report_replies);
    }

    return numberDuplicates > 0;
}

void process_image(Image &image, const std::string &url, int threadNumber) {
    std::string imageName = Image::determine_image_name(threadNumber);
    ApiWrapper::download_image(url, imageName);

    if (!Image::isValidImage(imageName)) {
        throw std::runtime_error("Invalid image format on " + url);
    }

    image.matrix = imread(Image::determine_image_name(threadNumber)); // TODO : If the matrix is empty, throw exception before extract_text
    image . extract_text(threadNumber);

    image.computeHash8x8();
    image . computeHash10x10();
}


bool handle_image(const Submission &submission, const SubredditSetting &settings, Image &image, const std::string &url, int threadNumber) {
    std::string commentContent;
    process_image(image, url, threadNumber);

    // If the image is completely uniform (e.g. only contains 1 color), then we don't even want to process it nor add it to the DB.
    if (image.getHash8x8().get_str() == "3069585302236515101")
        return true;

    bool submissionRemoved = search_image_duplicates( submission, settings, image, threadNumber);

    return submissionRemoved;
}

bool handle_link(const Submission &submission, const SubredditSetting &settings, int threadNumber) {
    std::string commentContent;
    auto query = interfaces[threadNumber]->get_link_rows( submission.subreddit);

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

bool handle_title(const Submission &submission, const SubredditSetting &settings, int threadNumber) {
    std::string commentContent;
    std::vector<std::string> reportComment; std::vector<std::string> removeComment;

    auto query = interfaces[threadNumber]->get_title_rows( submission.subreddit, settings.min_title_length_to_enforce);

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

void import_submissions(const std::string &subreddit, int threadNumber) {
    for (const std::string &range : {"month", "year", "all"}) {
        cpr::Response submissionQuery = apiWrapper.fetch_top_submissions(subreddit, range);

        json submissionList = json::parse( submissionQuery . text )[ "data" ][ "children" ];

        for (auto &submissionIter : submissionList) {
            try {
                Submission submission(submissionIter[ "data" ]);
                if (interfaces[threadNumber]->is_submission_saved( submission.subreddit, submission.id))
                    continue;
                else
                    interfaces[threadNumber]->save_submission( submission.subreddit, submission.id);

                if (submission.type == IMAGE || submission.type == VIDEO) {
                    Image image; // TODO: Code repeated here... figure out a way to plug-in a function
                    if (submission.isGallery) {
                        for (const std::string &url : submission.galleryUrls)
                        {
                            process_image(image, url, threadNumber);
                            interfaces[threadNumber]->insert_submission( submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string( image.get_dimensions()), submission.created, submission.type == LINK, submission.title, url);
                        }
                    } else {
                        process_image(image, submission.url, threadNumber);
                        interfaces[threadNumber]->insert_submission( submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string( image.get_dimensions()), submission.created, submission.type == LINK, submission.title, submission.url);
                    }
                }
                else {
                    interfaces[threadNumber]->insert_submission( submission.subreddit, "", "", "", submission.id, submission.author, "", submission.created, submission.type == LINK, submission.title, submission.url);
                }
            } catch (std::exception &e) {
                std::cerr << "IMPORT SUBMISSIONS:" << e.what() << std::endl;
            }
        }
    }
}

void insert_submission(const Submission &submission, Image &image, bool isMedia, int threadNumber) {
    if (isMedia) {
        if ( submission . isGallery )
        {
            for ( const auto &url : submission . galleryUrls )
                interfaces[threadNumber]->insert_submission( submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string( image.get_dimensions()), submission.created, submission.type == LINK, submission.title, url);
        } else
            interfaces[threadNumber]->insert_submission( submission.subreddit, image.ocrText, image.getHash10x10().get_str(), image.getHash8x8().get_str(), submission.id, submission.author, dimensions_to_string( image.get_dimensions()), submission.created, submission.type == LINK, submission.title, submission.url);
    }
    else // link or selftext
        interfaces[threadNumber]->insert_submission( submission.subreddit, "", "", "", submission.id, submission.author, "", submission.created, submission.type == LINK, submission.title, submission.url);
}

void process_submission(bool *finished, std::set<std::string> *sub2thread, std::set<std::string> *requiredOrder, const Submission &submission, const SubredditSetting &settings, int threadNumber) {
    try {
        interfaces[threadNumber]->save_submission( submission . subreddit, submission . id );

        std::cout << "Thread " << threadNumber << " for '" << submission.title << "' and sub " << submission.subreddit << std::endl;
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
            submissionRemoved = handle_link( submission, settings, threadNumber);

        // 3 - Submission is either a media or a link, but no duplicates have been found   ;  search for titles
        if ( !submissionRemoved && settings . enforce_titles && submission . title . size() >= settings . min_title_length_to_enforce )
        {
            submissionRemoved = handle_title( submission, settings, threadNumber );
        }

        if (!submissionRemoved)
            insert_submission(submission, image, isMedia, threadNumber);

    } catch (std::exception &e) {
        std::cout << "Thread " << threadNumber << " for '" << submission.title << "' and sub " << submission.subreddit << std::endl;
        std::cerr << "EXCEPTION ON THREAD NUMBER " << threadNumber << ":" << e.what() << std::endl;
    }

    // Remove blockers
    setMutex.lock();
    sub2thread->erase(submission.subreddit);
    requiredOrder->insert(submission.subreddit); // sure, we've freed the sub, but not until the loop has started over
    setMutex.unlock();

    std::cout << "Finished" << std::endl;
    *finished = true;
}

// Choosing the next thread in which to process a submission of the subreddit corresponding to 'sub'
[[nodiscard]] int choose_thread(const std::string &sub, const bool finished[NUMBER_THREADS], std::set<std::string> &sub2thread, std::set<std::string> &requiredOrder) {
    // If a thread with the same sub is already running, then wait. If a sub has finished in the current iteration, then wait as well.
    setMutex.lock();
    if (sub2thread.contains(sub) || requiredOrder.contains(sub))
    {
        setMutex.unlock();
        return -1;
    }
    setMutex.unlock();

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
            if ( !interfaces[NUMBER_THREADS]->is_submission_saved( submission . subreddit, submission . id ))
            {
                submissionList.push_back(submission);
            }

        } catch ( std::exception &e )
        {
            std::cerr << "EXCEPTION ON LIST-BUILDING:" << e.what() << " : " << submissionIter . value()[ "data" ] << std::endl;
        }
    }
}


void iterate_submissions() {
    cpr::Response submissionQuery = apiWrapper . fetch_submissions();

    json submissionJSON = json::parse( submissionQuery . text )[ "data" ][ "children" ];

    std::list<Submission> submissionList;

    populate_list(submissionList, submissionJSON);

    benchmarkThreads.reserve(NUMBER_THREADS);

    bool finished[NUMBER_THREADS]; for (bool & i : finished) i = true;
    bool alreadyCalled[NUMBER_THREADS]; for (bool & i : alreadyCalled) i = false;

    std::set<std::string> sub2thread; // Indicates if a given subreddit has a submission being treated in a thread

    // If a subreddit is present, indicates that it has been freed by a thread, but we have to wait for the loop to start over
    // before sending a new submission of the same subreddit in a thread.
    // The reason behind is that we want submissions to be treated in order (from the oldest submission to the newest one).
    std::set<std::string> requiredOrder;

    while (!submissionList.empty()) {

        setMutex.lock();
        requiredOrder.clear();
        setMutex.unlock();

        for (auto it = submissionList.begin(); it != submissionList.end(); )
        {
            auto &submission = *it;

            RowResult settingsQuery = interfaces[NUMBER_THREADS]->get_subreddit_settings( submission . subreddit );
            SubredditSetting settings{}; // Temporarily initializing it here so the compiler doesn't freak out
            try {
                settings = SubredditSetting( settingsQuery );
            } catch( std::runtime_error &e) { // Exception is thrown when a setting is somehow of invalid format
                std::cerr << "Invalid setting format on " << submission.subreddit << std::endl;
                interfaces[NUMBER_THREADS]->add_settings_row(submission.subreddit);
                continue;
            }

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
                setMutex.lock();
                sub2thread.insert(submission.subreddit); // the subreddit is busy
                setMutex.unlock();

                // Calling join() on a never-initialized thread will cause exceptions, hence this IF
                if (!alreadyCalled[thread])
                    alreadyCalled[thread] = true;
                else {
                    std::cout << "Joined " << thread << std::endl; // TODO: REMOVE
                    benchmarkThreads[thread]->join();
                    delete benchmarkThreads[thread];
                }
                benchmarkThreads[thread] = new std::thread(process_submission, finished + thread, &sub2thread, &requiredOrder, submission, settings, thread);
                //sleep(15);
                next_iteration(it, submissionList); // Remove the submission from the list and increment it
            }
            else
                it++;
        }
    }
    for (int i = 0; i < NUMBER_THREADS; i++) {
        if (alreadyCalled[i]) {
            benchmarkThreads[i]->join();
            delete benchmarkThreads[i];
            std::cout << "Joined " << i << std::endl; // TODO: REMOVE
        }
    }
}
