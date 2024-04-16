#pragma once

#include <httplib.h>

#include <string>
#include <vector>
#include <memory>

#include "Answers.hpp"
#include "Journals.hpp"
#include "Predictions.hpp"
#include "Questions.hpp"
#include "Settings.hpp"
#include "Users.hpp"


class Api {
 public:
    Api(std::string dbPath, int port){
        std::shared_ptr<httplib::Server> server = make_shared<httplib::Server>();
        Database db(dbPath);
        std::vector<std::shared_ptr<Route>> routes = {
            std::make_shared<Answers>(db, server),
            std::make_shared<Journals>(db, server),
            std::make_shared<Predictions>(db, server),
            std::make_shared<Questions>(db, server),
            std::make_shared<Settings>(db, server),
            std::make_shared<Users>(db, server)
        };
        for (auto route : routes) {
            route->init();
        }
        server->set_logger([](httplib::Request req, const Response& res) {
            std::cout << "--------------- Got request! ---------------" << std::endl <<
                "Status:   " << (res.status == 200 ? "\033[38;2;0;255;0m" : "\033[38;2;255;0;0m") << res.status << "\033[0m" << std::endl <<
                "Path:     " << req.path << std::endl <<
                "req Body: " << req.body << std::endl <<
                "res Body: " << res.body << std::endl <<
                "Params:   " << std::endl;
            for (auto [key, value] : req.path_params)
                std::cout << "\t" << key << ": " << value << std::endl;
        });
        server->listen("localhost", port);
    }
};
