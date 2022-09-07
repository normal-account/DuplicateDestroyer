//
// Created by carle on 8/29/22.
//

#include "db_interface.h"


std::shared_ptr<std::vector<mpz_class>> db_interface::get_hashes(const std::string& hash_type) {
    auto hashes = std::make_shared<std::vector<mpz_class>>();
    auto table = db->getTable(subreddit);
    auto query = table.select(hash_type).where("8pxhash is not null and 10pxhash is not null and ocr_string is not null");

    auto result = query.execute().begin().operator*().get(0);
    for (const auto& it : result) {
        mpz_class mpzHash;
        mpzHash.set_str(it.get<std::string>(), 10);
        hashes->push_back(mpzHash);
    }
    return hashes;
}

std::shared_ptr<std::vector<std::string>> db_interface::get_ocr_strings()
{
    auto ocr_strings = std::make_shared<std::vector<std::string>>();
    auto table = db->getTable(subreddit);
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

mysqlx::internal::Iterator<mysqlx::internal::Row_result_detail<mysqlx::abi2::r0::Columns>, mysqlx::Row> db_interface::get_subreddit_settings(const std::string &name) {
    auto table = db->getTable("SubredditSettings");
    return table.select("*").where("'subreddit' = " + name).execute().begin();
}





