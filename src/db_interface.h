#ifndef DUPLICATEDESTROYER_DB_INTERFACE_H
#define DUPLICATEDESTROYER_DB_INTERFACE_H
#include <iostream>
#include <mysqlx/xdevapi.h>
#include <memory>
#include <utility>
#include <gmpxx.h>

using namespace ::mysqlx;
class db_interface
{
private:
   Session session;
   std::string subreddit;
   std::shared_ptr<std::vector<mpz_class>> get_hashes(const std::string &hash_type);

public:
    db_interface( Session newSession ) : session(std::move( newSession ))
    {
        auto db = session.getSchema("all_reposts");
        auto table = db.getTable("SubredditSettings", true);
        std::cout << table.count() << std::endl;
    }

    std::shared_ptr<std::vector<mpz_class>> get_8x8_hashes();

    RowResult get_subreddit_settings(const std::string &name);

    std::shared_ptr<std::vector<mpz_class>> get_10x10_hashes();

    std::shared_ptr<std::vector<std::string>> get_ocr_strings();

    void insert_submission( const std::string &ocrtext, const std::string &tenpx, const std::string &eightpx, const std::string &id, const std::string &author,
                            const std::string &dimensions, long long int date, bool isVideo, const std::string &title, const std::string &url );


    std::shared_ptr<RowResult> get_image_rows();

    std::shared_ptr<RowResult> get_link_rows();

    std::shared_ptr<RowResult> get_title_rows(int minTitleLength);

    void switch_subreddit(std::string sub) {
        this->subreddit = std::move(sub);
    }

    bool subreddit_table_exists(const std::string &sub);

    void update_subreddit_settings(const std::string& sub, const std::string &parameter, const std::string &value);

    void add_settings_row(const std::string& sub);
};


#endif //DUPLICATEDESTROYER_DB_INTERFACE_H
