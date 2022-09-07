#ifndef DUPLICATEDESTROYER_DB_INTERFACE_H
#define DUPLICATEDESTROYER_DB_INTERFACE_H
#include <iostream>
#include <mysqlx/xdevapi.h>
#include <memory>
#include <gmpxx.h>
using namespace ::mysqlx;

class db_interface
{
private:
   std::shared_ptr<Schema> db;
   std::string subreddit;
   std::shared_ptr<std::vector<mpz_class>> get_hashes(const std::string &hash_type);

public:
    db_interface(const std::string &url, int port, const std::string &username, const std::string &password) {
        Session sess(url, port, username, password);
        db = std::make_shared<Schema>(sess.getSchema("all_reposts"));
        std::cout << db->getTables().begin().operator*().getName() << std::endl;
    }

    std::shared_ptr<std::vector<mpz_class>> get_8x8_hashes();

    std::shared_ptr<std::vector<mpz_class>> get_10x10_hashes();

    void switch_subreddit(std::string sub) {
        this->subreddit = sub;
    }

};


#endif //DUPLICATEDESTROYER_DB_INTERFACE_H
