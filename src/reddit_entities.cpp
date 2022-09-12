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
    created = data[ "created" ];
    shortlink = "https://redd.it/";
    shortlink.append( id);


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

 SubredditSetting::SubredditSetting( RowResult &settingRows)
 {
    // Conversions needed here
    auto settings = settingRows.begin();
    enabled = (bool)(*settings).get(1);
    imported = (bool)(*settings).get(2);
    remove_threshold = (int)(*settings).get(3);
    report_threshold = (int)(*settings).get(4);
    enforce_videos = (bool)(*settings).get(5);
    enforce_images = (bool)(*settings).get(6);
    enforce_links = (bool)(*settings).get(7);
    time_range = (int)(*settings).get(8);
    report_links = (bool)(*settings).get(9);
    report_replies = (bool)(*settings).get(10);
    enforce_titles = (bool)(*settings).get(11);
    title_remove_threshold = (int)(*settings).get(12);
    title_report_threshold = (int)(*settings).get(13);
    min_title_length_to_enforce = (int)(*settings).get(14);
    removal_table_duplicate_number = (int)(*settings).get( 15);
    ocr_text_threshold = (int)(*settings).get(16);
 }