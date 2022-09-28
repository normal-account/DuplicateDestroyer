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
    isGallery = !data[ "is_gallery" ].is_null();
    created = data[ "created" ];
    shortlink = "https://redd.it/";
    shortlink.append(id);
    url = data[ "url" ];
    saved = data[ "saved" ];
    if (isGallery) {
        type = IMAGE;
        json urlList = data["media_metadata"];
        for ( auto urlIter = urlList.begin(); urlIter != urlList.end(); urlIter++) {
            std::string submissionID = urlIter.value()["id"];
            std::string fileType = urlIter.value()["m"]; // e.g. for a JPG, "m" gives "image/jpg"
            fileType = fileType.substr(6, fileType.size() - 6); // Extracting only the datatype (excluding 'image/')
            std::string constructedURL;
            constructedURL.append("i.redd.it/").append( submissionID).append( ".").append( fileType );
            galleryUrls.push_back(constructedURL);
        }
    } else
    {
        if ( is_image())
        {
            type = IMAGE;
        } else if ( data[ "is_video" ] || is_video())
        {
            type = VIDEO;
            url = data[ "thumbnail" ];
        } else
            type = LINK;
    }
}

bool Submission::is_image() const
{

    return (url.ends_with(".jpg") || url.ends_with(".jpg?1") || url.ends_with(".png") || url.ends_with("png?1")
            || url.ends_with(".jpeg") || url.find("reddituploads.com") != std::string::npos || url.find("reutersmedia.net") != std::string::npos
            || url.find("500px.org") != std::string::npos || url.find("redditmedia.com") != std::string::npos
    );
}

bool Submission::is_video() const
{
    return (url.ends_with(".gif") || url.ends_with(".mp4") || url.ends_with(".gifv") || url.find("v.redd.it") != std::string::npos
            || url.find("redgifs.com") != std::string::npos || url.find("youtu.be") != std::string::npos || url.find("youtube.com") != std::string::npos
            || url.find("giphy.com") != std::string::npos || url.find("gfycat.com") != std::string::npos || url.find("liveleak.com") != std::string::npos
            || url.find("streamable.com") != std::string::npos);
}

Message::Message( json data ) {
    type = data["kind"];
    id = data["data"]["id"];
    isReply = type == "t1";// "kind" returns type, e.g. "T1" or "T4"
    fullname = (type == "unknown" ? "t4_" : type + "_") + id;
    subject = data[ "data" ][ "subject" ];
    body = data[ "data" ][ "body" ];
    if (!data[ "data" ][ "author" ].is_null()) {
        author.emplace();
        author = data[ "data" ][ "author" ];
    }
    if (!data[ "data" ][ "subreddit" ].is_null()) {
        subreddit.emplace();
        subreddit = data[ "data" ][ "subreddit" ];
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