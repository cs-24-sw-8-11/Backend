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
#include "Mitigations.hpp"
#include "Tests.hpp"

#include "Globals.hpp"

using namespace std;
using namespace httplib;
using namespace P8;

class Api {
 public:
    /// @brief Constructs the API with a give path to the database file and webserver port.
    /// @param dbPath
    /// @param port
    Api(string dbPath, int port){
        shared_ptr<Server> server = make_shared<Server>();
        Database db(dbPath);
        vector<std::shared_ptr<Route>> routes = {
            make_shared<Answers>(db, server),
            make_shared<Journals>(db, server),
            make_shared<Predictions>(db, server),
            make_shared<Questions>(db, server),
            make_shared<Settings>(db, server),
            make_shared<Users>(db, server),
            make_shared<Mitigations>(db, server),
            make_shared<Tests>(db, server)
        };
        for (auto route : routes) {
            route->init();
            log<DEBUG>("Initialized route: {}", typeid(*route).name());
        }

        log<IMPORTANT>("Initialized routes and endpoints");
        /// @brief Initializes the logger that prints out every request.
        server->set_logger([](httplib::Request req, const Response& res) {
            string red = "\033[38;2;255;0;0m";
            string green = "\033[38;2;0;255;0m";
            log("--------------- Got request! ---------------");
            log("Status:   {}{}\033[0m", (res.status >= 200 && res.status < 300 ? green : red), res.status);
            log<IMPORTANT>("Path:     {}", req.path);
            log<IMPORTANT>("req Body: {}", req.body.c_str());
            log("res Body: {}", res.body.c_str());
            log<IMPORTANT>("Params:   ");
            for (auto [key, value] : req.path_params) {
                log<IMPORTANT>("\t{}:{}", key, value);
            }
        });
        log<DEBUG>("Initialized logger");
        log("Starting server on localhost: {}", port);
        server->listen("localhost", port);
    }
};
