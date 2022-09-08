//
// Created by carle on 3/14/22.
//

#include "reddit_entities.h"



Submission::Submission( json data )
{
    subreddit = data[ "subreddit" ];
    author = data[ "author" ];
    title = data[ "title" ];
    fullname = data[ "name" ];
    id = data[ "id" ];
    score = data[ "score" ];
    isVideo = data[ "is_video" ];
    isGallery = !data[ "is_gallery" ].is_null();
    shortlink = "redd.it/";
    shortlink.append( fullname.replace(0, 3, "", 0, 0) ); // Getting the id from the fullname


    if (isVideo)
        url = data[ "thumbnail" ];
    else {
        if (isGallery) {
            json urlList = data["media_metadata"];
            for ( auto urlIter = urlList.begin(); urlIter != urlList.end(); urlIter++) {
                std::string submissionID = urlIter.value()["id"];
                std::string type = urlIter.value()["m"]; // e.g. for a PNG, "m" gives "image/jpg"
                type = type.substr(6, type.size() - 6); // Extracting only the datatype (excluding 'image/')
                std::string constructedURL;
                constructedURL.append("i.redd.it/").append( submissionID).append( ".").append( type);
                galleryUrls.push_back(constructedURL);
            }
        }
        else
            url = data[ "url" ];
    }
}

Message::Message( json data ) {
    isReply = data["kind"] == "t1";// "kind" returns type, e.g. "T1" or "T4"
    if (!isReply)
    {
        author = data[ "data" ][ "author" ];
        subject = data[ "data" ][ "subject" ];
        body = data[ "data" ][ "body" ];
    }
}

 SubredditSetting::SubredditSetting(const mysqlx::internal::Iterator<mysqlx::internal::Row_result_detail<mysqlx::abi2::r0::Columns>, mysqlx::Row> &settings)
 {
    // Conversions needed here
    enabled = (bool)(*settings).get(1);
    remove_threshold = (int)(*settings).get(2);
    report_threshold = (int)(*settings).get(3);
    enforce_videos = (bool)(*settings).get(4);
    enforce_images = (bool)(*settings).get(5);
    enforce_links = (bool)(*settings).get(6);
    time_range = (int)(*settings).get(7);
    report_links = (bool)(*settings).get(8);
    report_replies = (bool)(*settings).get(9);
    enforce_titles = (bool)(*settings).get(10);
    title_remove_threshold = (int)(*settings).get(11);
    title_report_threshold = (int)(*settings).get(12);
    min_title_length_to_enforce = (int)(*settings).get(13);
    removal_table_duplicate_number = (int)(*settings).get(14);
    ocr_text_threshold = (int)(*settings).get(15);
 }