#pragma once

#include <memory>
#include <httplib.h>

#include "Database.hpp"
#include "PredictionManager.hpp"

#define CROW_PTR_ROUTE(app, url) app->template route<crow::black_magic::get_parameter_tag(url)>(url)

class Route {
 protected:
    std::shared_ptr<Database> db;
    std::shared_ptr<httplib::Server> server;
    std::map<int, std::string> authedUsers;
    PredictionManager manager;

    void DefaultSettings(int userId) {
        db->settings->add({
            "key",
            "value",
            "userId"}, {
            "key",
            "value",
            db_int(userId)});
        db->settings->add({
            "key",
            "value",
            "userId"}, {
            "key1",
            "value2",
            db_int(userId)});
        // add more settings here
    }

    int UserIdFromToken(std::string token) {
        for (auto [uid, utoken] : authedUsers) {
            if (utoken == token) {
                return uid;
            }
        }
        return 0;
    }
    bool SettingExists(std::vector<int> ids, std::string key) {
        for (auto id : ids) {
            auto row = db->settings->get(id);
            if (row["key"] == key) {
                return true;
            }
        }
        return false;
    }


 public:
    Route(Database db, std::shared_ptr<httplib::Server> server){
        this->db = std::make_shared<Database>(db);
        this->server = server;
    }
    virtual void init(){
        std::cerr << "Error, called unimplemented run method" << std::endl;
        throw std::exception();
    }
};