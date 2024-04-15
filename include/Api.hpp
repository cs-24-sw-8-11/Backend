#include "Answers.hpp"
#include "Journals.hpp"
#include "Predictions.hpp"
#include "Questions.hpp"
#include "Settings.hpp"
#include "Users.hpp"

#include <crow.h>

class Api {
 public:
    Api(std::string dbPath, int port){
        crow::SimpleApp app;
        Database db(dbPath);
        std::vector<std::shared_ptr<Route>> routes = {
            std::make_shared<Answers>(db, app),
            std::make_shared<Journals>(db, app),
            std::make_shared<Predictions>(db, app),
            std::make_shared<Questions>(db, app),
            std::make_shared<Settings>(db, app),
            std::make_shared<Users>(db, app)
            
        };
        for(auto route : routes){
            route->init();
        }
        app.port(port).multithreaded().run();
    }
};