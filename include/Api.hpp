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
#include "Database.hpp"

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
        server->set_logger([](const auto& req, const auto& res) {
            string red = "\033[38;2;255;0;0m";
            string green = "\033[38;2;0;255;0m";
            stringstream ss;
            ss << "--------------- Got request! ---------------" << endl <<
                "Status:   " << (res.status >= 200 && res.status < 300 ? green : red) << res.status << "\033[0m" << endl <<
                "Path:     " << req.path << endl <<
                "req Body: " << req.body << endl <<
                "res Body: " << res.body << endl <<
                "Params:   " << endl;
            for (auto [key, value] : req.path_params)
                ss << "\t" << key << ": " << value << endl;
            log("{}", ss.str());
        });
        log<DEBUG>("Initialized logger");

        server->set_exception_handler([](const auto& req, auto& res, std::exception_ptr e_ptr){
            json response_data;
            try{
                rethrow_exception(e_ptr);
            }
            catch(DbException& e){
                loge("Database exception!");
                loge("why: {}", e.what());
                response_data["error"] = e.what();
            }
            catch (exception& e){
                loge("Unknown exception");
                loge("why: {}", e.what());
                response_data["error"] = e.what();
            }
            res.set_content(to_string(response_data), "application/json");
            res.status = 500;
        });

        log<DEBUG>("Initialized exception handler");

        log("Starting server on localhost: {}", port);
        server->listen("localhost", port);
    }
};
