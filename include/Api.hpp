#pragma once
#include <format>
#include <crow.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <utility>

#include "Endpoints.hpp"

#include "Endpoints/Users.hpp"
#include "Endpoints/Answers.hpp"
#include "Endpoints/Journals.hpp"
#include "Endpoints/Questions.hpp"
#include "Endpoints/Settings.hpp"
#include "Endpoints/Predictions.hpp"

class API {
 public:
    crow::SimpleApp app;
    std::map<int, std::string> authedUsers;
    std::shared_ptr<Database> db;
    std::shared_ptr<PredictionManager> manager;

    explicit API(std::string path) {
        db = std::make_shared<Database>(path);
        manager = std::make_shared<PredictionManager>();
        auto users = Users(&app, &authedUsers, db, manager);

    }
    void Run(int port) {
        app.port(port).multithreaded().run();
    }
    private:

};
