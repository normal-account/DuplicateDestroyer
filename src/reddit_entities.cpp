//
// Created by carle on 3/14/22.
//

#include "reddit_entities.h"

void Submission::operator<<( json data )
{
    author = data[ "author" ];
    title = data[ "title" ];
    fullname = data[ "name" ];
    score = data[ "score" ];
    isVideo = data[ "is_video" ];
    isGallery = data[ "is_gallery" ];
    shortlink = "redd.it/";
    shortlink.append( fullname.replace(0, 3, "", 0, 0) ); // Getting the id from the fullname


    if (isVideo)
        url = data[ "thumbnail" ];
    else {
        if (isGallery) {
            galleryUrls.emplace();
            json urlList = data["media_metadata"];
            for ( auto urlIter = urlList.begin(); urlIter != urlList.end(); urlIter++) {
                string id = urlIter.value()["id"];
                string type = urlIter.value()["m"]; // e.g. for a PNG, "m" gives "image/jpg"
                type = type.substr(6, type.size() - 6); // Extracting only the datatype (excluding 'image/')
                string constructedURL;
                constructedURL.append("i.redd.it/").append(id).append(".").append(type);
                galleryUrls->push_back(constructedURL);
            }
        }
        else
            url = data[ "url" ];
    }
}

void Message::operator<<( json data ) {
    isReply = data["kind"] == "t1";// "kind" returns type, e.g. "T1" or "T4"
    if (!isReply)
    {
        author = data[ "data" ][ "author" ];
        subject = data[ "data" ][ "subject" ];
        body = data[ "data" ][ "body" ];
    }
}