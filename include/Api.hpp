#include "Answers.hpp"
#include "Journals.hpp"
#include "Predictions.hpp"
#include "Questions.hpp"
#include "Settings.hpp"
#include "Users.hpp"

#include <httplib.h>

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
        for(auto route : routes){
            route->init();
        }
        server->listen("localhost", port);
    }
};