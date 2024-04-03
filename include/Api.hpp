#include <crow.h>

#include <iostream>
#include <format>
#include <string>
#include "Database.hpp"
#include <nlohmann/json.hpp>

class API {
   public:
    crow::SimpleApp app;
    std::shared_ptr<Database> db;
    API(std::string path, int port) {
        db = std::make_shared<Database>(path);

        CROW_ROUTE(app, "/db")([&]{
            auto users = db->users;
            auto size = users->get_where("username","taoshi");
            auto user = users->get(size.front());
            crow::json::wvalue x({});
            x["password"] = user.begin()->first;
            x["username"] = user.begin()->first;
            return x;
        });

        CROW_ROUTE(app, "/user/get/<int>")
        ([&](int id) {
            crow::json::wvalue x({});
            auto user = db->users->get(id);
            if (id < 0){
                x["error"] = "Invalid id";
                return x;
            }
            x["username"] = user["username"];
            x["password"] = user["password"];
            x["id"] = user["id"];
            x["userdataId"] = user["userdataId"];
            x["stress level"] = 100;
            return x;
        });
        CROW_ROUTE(app, "/user/ids/<int>-<int>")
        ([](int min, int max) {
            crow::json::wvalue x({});
            if (min < 0 || max < 0 || (min-max <= 0)){
                x["error"] = "Invalid range";
                return x;
            }
            x["min"] = min;
            x["max"] = max;
            x["usercount"] = max-min;
            return x;
        });
        
        CROW_ROUTE(app, "/auth")
        .methods("POST"_method)
        ([](const crow::request& req) {
            auto x = crow::json::load(req.body);
            crow::json::wvalue responsejson({});
            if (!x){
                responsejson["error"] = req.body;
                return responsejson;
            }
        
            auto username = x["username"].s();
            auto password = x["password"].s();
            if(username == "tester" && password == "tester"){
                responsejson["status"] = "thomas authorized";

                return responsejson;
            }
            else{
                responsejson["error"] = "Invalid token";
                return responsejson;
            }
        });
        CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            crow::json::wvalue responsejson({});
            if (!x){
                responsejson["error"] = req.body;
                return responsejson;
            }
        
            auto username = x["username"].s();
            auto password = x["password"].s();
            if(username.size() > 0 && password.size() > 0){ // we dont want empty usernames and passwords
                std::vector<std::string> keys = {username};
                std::vector<std::string> values = {username, password};
                db->users->add(keys,values);
                responsejson["status"] = "Successfully Registered";
                responsejson["username"] = username;
                responsejson["password"] = password;

                return responsejson;
            }
            else{
                responsejson["error"] = "Invalid token";
                return responsejson;
            }
        });
        CROW_ROUTE(app, "/user/data/update")
        .methods("POST"_method)
        ([](const crow::request& req) {
            auto x = crow::json::load(req.body);
            crow::json::wvalue responsejson({});
            if (!x){
                responsejson["error"] = req.body;
                return responsejson;
            }
        
            auto token = x["token"].s();
            auto data = x["data"].s();
            if(token == "thomas"){
                responsejson["status"] = "thomas authorized";
                responsejson["data"] = data;

                return responsejson;
            }
            else{
                responsejson["error"] = "Invalid token";
                return responsejson;
            }
        });
        
        CROW_ROUTE(app, "/journals/new")
        .methods("POST"_method)
        ([](const crow::request& req) {
            auto x = crow::json::load(req.body);
            nlohmann::json loaded = nlohmann::json::parse(req.body);
            crow::json::wvalue responsejson({});
            if (!x){
                responsejson["error"] = req.body;
                return responsejson;
            }
            auto token = x["token"].s();
            auto data = x["data"];
            if(token == "thomas"){
                responsejson["status"] = "thomas authorized";
                std::string something = "New Journal Entry:\n";
                auto vector = loaded["data"].get<std::vector<std::string>>();
                for(auto i = 0; i < vector.size(); i ++){
                    something += std::format("{}\n",vector[i]);
                    //something += std::format("Question: {0}\nAnswer: {1}", data[i]["question"].s(), data[i]["answer"].s());
                }
                responsejson["data"] = something;

                return responsejson;
            }
            else{
                responsejson["error"] = "Invalid token";
                return responsejson;
            }
        });
        
        CROW_ROUTE(app, "/journals/ids/<int>")
        ([](int id) {
            crow::json::wvalue x({});
            if (id < 0){
                x["error"] = "Invalid id";
                return x;
            }
            x["id"] = id;
            x["journal"] = "thomas stress";
            return x;
        });
        /*
        CROW_ROUTE(app, "/journals/delete/<int>/<int>")
        .methods("DELETE"_method)
        ([](const crow::request& req, int userid, int journalid) {
            auto x = crow::json::load(req.body);
            crow::json::wvalue responsejson({});
            if (!x){
                responsejson["error"] = "Unable to load/parse json";
                return responsejson;
            }
            auto token = x["token"].s();
            auto data = x["data"];
            if(token == "thomas"){
                responsejson["status"] = "thomas authorized";
                std::string something = "New Journal Entry:\n";
                for(auto i = 0; i < 0; i ++){
                    something += std::format("Question: {}\nAnswer: {}", data[i]["question"].s(), data[i]["answer"].s());
                }
                responsejson["data"] = something;

                return responsejson;
            }
            else{
                responsejson["error"] = "Invalid token";
                return responsejson;
            }
        });
        */
        CROW_ROUTE(app, "/settings/get/<int>")
        ([&](int id) {
            crow::json::wvalue x({});
            if (id < 0){
                x["error"] = "Invalid id";
                return x;
            }
            auto settings = db->settings->get(id);
            for(auto iter = settings.begin(); iter != settings.end(); ++iter){
                x[iter->first] = iter->second;
            }
            return x;
        });
        CROW_ROUTE(app, "/questions/default")
        ([&]() {
            crow::json::wvalue x({});
            auto questions = db->questions->get_where("tags","default");

            for(auto i = 0; i > questions.size(); i++){
                auto question = db->questions->get(i);
                for(auto iter = question.begin(); iter != question.end(); ++iter){
                    x[iter->first] = iter->second;
                }
            }
            return x;
        });
        CROW_ROUTE(app, "/questions/get/<string>")
        ([&](std::string inputString) {
            crow::json::wvalue x({});
            auto questions = db->questions->get_where("tags",inputString);

            for(auto i = 0; i > questions.size(); i++){
                auto question = db->questions->get(i);
                for(auto iter = question.begin(); iter != question.end(); ++iter){
                    x[iter->first] = iter->second;
                }
            }
            return x;
        });



        app.port(port).multithreaded().run();
    }

   private:
};