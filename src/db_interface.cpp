//
// Created by carle on 8/29/22.
//

#include "db_interface.h"


std::shared_ptr<std::vector<mpz_class>> db_interface::get_hashes(const std::string& hash_type) {
    auto hashes = std::make_shared<std::vector<mpz_class>>();
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable(subreddit);
    auto query = table.select(/*hash_type*/"*").where("8pxhash is not null and 10pxhash is not null and ocr_string is not null");

    auto result = query.execute().begin().operator*().get(0);
    for (const auto& it : result) {
        mpz_class mpzHash;
        mpzHash.set_str(it.get<std::string>(), 10);
        hashes->push_back(mpzHash);
    }
    return hashes;
}


std::shared_ptr<RowResult> db_interface::get_image_rows() {
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable(subreddit);
    auto query = table.select("*").where("8pxhash is not null and 10pxhash is not null and ocr_string is not null");
    return std::make_shared<RowResult>(query.execute());
}

std::shared_ptr<RowResult> db_interface::get_link_rows() {
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable(subreddit);
    auto query = table.select("*").where("is_link = 1");
    return std::make_shared<RowResult>(query.execute());
}

std::shared_ptr<RowResult> db_interface::get_title_rows(int minTitleLength) {
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable(subreddit);
    auto query = table.select("*").where("length(title) > " + std::to_string(minTitleLength));
    return std::make_shared<RowResult>(query.execute());
}

void db_interface::insert_submission( const std::string &ocrtext, const std::string &tenpx, const std::string &eightpx, const std::string &id, const std::string &author,
                                      const std::string &dimensions, long long int date, bool isVideo, const std::string &title, const std::string &url)
{
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable(subreddit);
    table.insert().values(ocrtext, tenpx, eightpx, id, author, dimensions, date, isVideo, title, url).execute();
}

std::shared_ptr<std::vector<std::string>> db_interface::get_ocr_strings()
{
    auto db = session.getSchema("all_reposts");
    auto ocr_strings = std::make_shared<std::vector<std::string>>();
    auto table = db.getTable(subreddit);
    auto query = table.select("ocr_hash").where("8pxhash is not null and 10pxhash is not null and ocr_string is not null");
    return ocr_strings;
}

std::shared_ptr<std::vector<mpz_class>> db_interface::get_8x8_hashes()
{
    return get_hashes("8pxhash");
}


std::shared_ptr<std::vector<mpz_class>> db_interface::get_10x10_hashes()
{
    return get_hashes("10pxhash");
}


mysqlx::RowResult db_interface::get_subreddit_settings(const std::string &name) {
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable("SubredditSettings");
    std::string where = "subreddit = '" + name + "'";
    auto row = table.select("*").where(where).execute();
    assert(row.count());

    return row;
}

void db_interface::update_subreddit_settings(const std::string& sub, const std::string &parameter, const std::string &value) {
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable("SubredditSettings");
    std::string where = "subreddit = '" + sub + "'";
    table.update().set(parameter, value).where(where).execute();
}

void db_interface::add_settings_row( const std::string &sub )
{
    auto db = session.getSchema("all_reposts");
    auto table = db.getTable("SubredditSettings");
    // TODO : Revisit default values
    table.insert().values(sub, 1, 0, 95, 89, 1, 1, 1, 90, 0, 0, 0, 85, 95, 10, 5, 80).execute();
}







