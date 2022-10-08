#include <iostream>
#include <mysqlx/xdevapi.h>
#include <memory>
#include <utility>
#include <gmpxx.h>
#include <atomic>
#include <semaphore>

#ifndef DUPLICATEDESTROYER_DB_INTERFACE_H
#define DUPLICATEDESTROYER_DB_INTERFACE_H

using namespace ::mysqlx;
class db_interface
{
private:
   std::atomic<unsigned> newSubmissions = 0;
   Session session;
   std::shared_ptr<std::vector<mpz_class>> get_hashes(const std::string &subreddit, const std::string &hash_type);

public:
    db_interface( Session newSession ) : session(std::move( newSession ))
    {
        auto db = session.getSchema("all_reposts");
        auto table = db.getTable("SubredditSettings", true);
        std::cout << table.count() << std::endl;
    }

    std::shared_ptr<std::vector<mpz_class>> get_8x8_hashes(const std::string &subreddit);

    RowResult get_subreddit_settings(const std::string &name);

    std::shared_ptr<std::vector<mpz_class>> get_10x10_hashes(const std::string &subreddit);

    void insert_submission( const std::string &subreddit, const std::string &ocrtext, const std::string &tenpx, const std::string &eightpx, const std::string &id,
                            const std::string &author, const std::string &dimensions, long long int date, bool isVideo, const std::string &title, const std::string &url );

    std::shared_ptr<RowResult> get_image_rows(const std::string &subreddit);

    std::shared_ptr<RowResult> get_link_rows(const std::string &subreddit);

    std::shared_ptr<RowResult> get_title_rows(const std::string &subreddit, int minTitleLength);

    //void switch_subreddit(std::string sub) { this->subreddit = std::move(sub); }

    [[nodiscard]] bool settings_exist(const std::string &sub);

    bool subreddit_table_exists(const std::string &sub);

    void update_subreddit_settings(const std::string& sub, const std::string &parameter, const std::string &value);

    void add_settings_row(const std::string& sub);

    void create_table_duplicates( const std::string &sub);

    void create_table_saved( const std::string &sub);

    void save_submission( const std::string &sub, const std::string &id );

    bool is_submission_saved( const std::string &sub, const std::string &id );

    unsigned get_new_submissions();
};


#endif //DUPLICATEDESTROYER_DB_INTERFACE_H
