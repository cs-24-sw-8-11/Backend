#include <crow.h>

#include <iostream>
#include <string>
#include "Database.hpp"

class API {
   public:
    crow::SimpleApp app;
    std::shared_ptr<Database> db;
    API(std::string path, int port) {
        db = std::make_shared<Database>(path);

        CROW_ROUTE(app, "/db")([&]{
            auto users = db->users;
            auto size = users->get_where("username","taoshi");
            crow::json::wvalue x({});
            x["password"] = "thomas";
            x["username"] = "thomas";
            return x;
        });

        CROW_ROUTE(app, "/user/get/<int>")
        ([](int id) {
            if (id < 0)
                return crow::response(400);
            crow::json::wvalue x({});
            x["username"] = "thomas";
            x["id"] = id;
            x["stress level"] = 100;
            return x;
        });
        CROW_ROUTE(app, "/user/ids/<int>-<int>")
        ([](int min, int max) {
            if (min < 0 || max < 0 || (min-max <= 0))
                return crow::response(400);
            crow::json::wvalue x({});
            x["min"] = min;
            x["max"] = max;
            x["usercount"] = max-min;
            return x;
        });
        
        CROW_ROUTE(app, "/auth")
        .methods("POST"_method)
        ([](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(crow::status::BAD_REQUEST); // same as crow::response(400)
        
            auto username = x["username"].s();
            auto password = x["password"].s();
            if(username == "thomas" && password == "thomas"){
                crow::json::wvalue responsejson({});
                responsejson["something"] = "thomas authorized";

                return responsejson;
            }
            else{
                return crow::response(403);
            }
        });
        CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(crow::status::BAD_REQUEST); // same as crow::response(400)
        
            auto username = x["username"].s();
            auto password = x["password"].s();
            if(username.size() > 0 && password.size() > 0){ // we dont want empty usernames and passwords
                crow::json::wvalue responsejson({});
                std::vector<std::string> keys = {username};
                std::vector<std::string> values = {username, password};
                db->users->add(keys,values);
                responsejson["status"] = "successfully registered";
                responsejson["username"] = username;
                responsejson["password"] = password;

                return responsejson;
            }
            else{
                return crow::response(403);
            }
        });
        CROW_ROUTE(app, "/user/data/update")
        .methods("POST"_method)
        ([](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(crow::status::BAD_REQUEST); // same as crow::response(400)
        
            auto token = x["token"].s();
            auto data = x["data"].s();
            if(token == "thomas"){
                crow::json::wvalue responsejson({});
                responsejson["status"] = "thomas authorized";
                responsejson["data"] = data;

                return responsejson;
            }
            else{
                return crow::response(403);
            }
        });
        CROW_ROUTE(app, "/journals/new")
        .methods("POST"_method)
        ([](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(crow::status::BAD_REQUEST); // same as crow::response(400)
        
            auto token = x["token"].s();
            auto data = x["data"];
            if(token == "thomas"){
                crow::json::wvalue responsejson({});
                responsejson["status"] = "thomas authorized";
                std::string something = "New Journal Entry:\n";
                for(auto i = 0; i < 0; i ++){
                    something += std::format("Question: {}\nAnswer: {}", data[i]["question"], data[i]["answer"]);
                }
                responsejson["data"] = something;

                return responsejson;
            }
            else{
                return crow::response(403);
            }
        });
        CROW_ROUTE(app, "/journals/ids/<int>")
        ([](int id) {
            if (id < 0)
                return crow::response(400);
            crow::json::wvalue x({});
            x["id"] = id;
            x["journal"] = "thomas stress";
            return x;
        });
        CROW_ROUTE(app, "/journals/delete/<int>/<int>")
        .methods("DELETE"_method)
        ([](int userid, int journalid, const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(crow::status::BAD_REQUEST); // same as crow::response(400)
        
            auto token = x["token"].s();
            auto data = x["data"];
            if(token == "thomas"){
                crow::json::wvalue responsejson({});
                responsejson["status"] = "thomas authorized";
                std::string something = "New Journal Entry:\n";
                for(auto i = 0; i < 0; i ++){
                    something += std::format("Question: {}\nAnswer: {}", data[i]["question"], data[i]["answer"]);
                }
                responsejson["data"] = something;

                return responsejson;
            }
            else{
                return crow::response(403);
            }
        });
        app.port(port).multithreaded().run();
    }

   private:
};