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

#include "Globals.hpp"

using namespace std;
using namespace httplib;

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
            make_shared<Users>(db, server)
        };
        for (auto route : routes) {
            route->init();
            if (VERBOSE)
                cout << "Initialized route: " << typeid(*route).name() << endl;
        }
        if (VERBOSE)
            cout << "Initialized routes and endpoints" << endl;
        /// @brief Initializes the logger that prints out every request.
        server->set_logger([](httplib::Request req, const Response& res) {
            string red = "\033[38;2;255;0;0m";
            string green = "\033[38;2;0;255;0m";
            std::cout << "--------------- Got request! ---------------" << endl <<
                "Status:   " << (res.status >= 200 && res.status < 300 ? green : red) << res.status << "\033[0m" << endl <<
                "Path:     " << req.path << endl <<
                "req Body: " << req.body << endl <<
                "res Body: " << res.body << endl <<
                "Params:   " << endl;
            for (auto [key, value] : req.path_params)
                cout << "\t" << key << ": " << value << endl;
        });
        if (VERBOSE)
            cout << "Initialized logger" << endl;
        cout << "Starting server on localhost:" << port << endl;
        server->listen("localhost", port);
    }
};
