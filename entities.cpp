//
// Created by carle on 3/14/22.
//

#include "entities.h"

void Submission::operator<<( json data )
{
    author = data[ "author" ];
    title = data[ "title" ];
    fullname = data[ "name" ];
    score = data[ "score" ];
    isVideo = data[ "is_video" ];
    shortlink = "redd.it/";
    shortlink.append( fullname.replace(0, 3, "", 0, 0) ); // Getting the id from the fullname

    if (isVideo)
        url = data[ "thumbnail" ];
    else
        url = data[ "url" ];
}

void Message::operator<<( json data ) {
    if (data["kind"] == "t1") // "kind" returns type, e.g. "T1" or "T4"
        isReply = true;
    else
    {
        isReply = false;
        author = data[ "data" ][ "author" ];
        subject = data[ "data" ][ "subject" ];
        body = data[ "data" ][ "body" ];
    }
}