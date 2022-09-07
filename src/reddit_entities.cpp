//
// Created by carle on 3/14/22.
//

#include "reddit_entities.h"

void Submission::operator<<( json data )
{
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
                string id = urlIter.value()["id"];
                string type = urlIter.value()["m"]; // e.g. for a PNG, "m" gives "image/jpg"
                type = type.substr(6, type.size() - 6); // Extracting only the datatype (excluding 'image/')
                string constructedURL;
                constructedURL.append("i.redd.it/").append(id).append(".").append(type);
                galleryUrls.push_back(constructedURL);
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



// To consider a modified version of the Levenshtein algorithm that takes string length in consideration
int Submission::getEditDistance(const std::string &first, const std::string& second)
{
    int m = first.length();
    int n = second.length();

    int T[m + 1][n + 1];
    for (int i = 1; i <= m; i++) {
        T[i][0] = i;
    }

    for (int j = 1; j <= n; j++) {
        T[0][j] = j;
    }

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            int weight = first[i - 1] == second[j - 1] ? 0: 1;
            T[i][j] = std::min(std::min(T[i-1][j] + 1, T[i][j-1] + 1), T[i-1][j-1] + weight);
        }
    }

    return T[m][n];
}

double Submission::findStringSimilarity(const std::string &first, const std::string &second) {
    double max_length = std::max(first.length(), second.length());
    if (max_length > 0) {
        return ((max_length - getEditDistance(first, second)) / max_length ) * 100;
    }
    return 100.0;
}