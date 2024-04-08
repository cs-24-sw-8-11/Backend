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
    std::map<int, std::string> authedUsers;
    API(std::string path, int port) {
        db = std::make_shared<Database>(path);

        CROW_ROUTE(app, "/user/get/<int>")
        ([&](int id) {
            crow::json::wvalue x({});
            auto user = db->users->get(id);
            auto userdata = db->userdata->get(db->userdata->get_where("userId", std::format("{}",id)).front());
            if (id < 0){
                x["error"] = "Invalid id";
                return x;
            }
            x["username"] = user["username"];
            x["password"] = user["password"];
            x["id"] = user["id"];
            x["userdataId"] = user["userdataId"];
            x["stress level"] = 100;
            x["agegroup"] = userdata["agegroup"];
            x["occupation"] = userdata["occupation"];
            return x;
        });
        CROW_ROUTE(app, "/user/ids/<int>-<int>")
        ([&](int min, int max) {
            crow::json::wvalue x;
            if (min < 0 || max < 0 || (max-min < 0)){
                x["difference"] = max-min;
                x["max"] = max;
                x["min"] = min;
                x["error"] = "Invalid Range";
                return x;
            }
            auto users = db->users->get_where();
            std::vector<int> filteredUsers;
            for(const auto & user : users) {
                if(user <= max && user >= min) {
                    filteredUsers.push_back(user);
                }
            }
            x = std::move(filteredUsers); //Funky solution to make it only return json array
            return std::move(x);          //This is done due to "inconsistent type" errors appearing if not done
        });
        CROW_ROUTE(app, "/user/ids")
        ([&]() {
            crow::json::wvalue x;
            auto users = db->users->get_where();
            x = std::move(users);
            return std::move(x);
        });
        
        CROW_ROUTE(app, "/user/auth")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            //crow::json::wvalue responsejson({});
            if (!x){
                return crow::response(400, "Unable to load/parse json");
            }
        
            auto username = x["username"].s();
            auto password = x["password"].s();
            auto dbpassword = db->users->get(db->users->get_where("username", username).front())["password"];
            if(dbpassword == password){
                auto userid = db->users->get_where("username", username).front();
                auto hash = Hash((std::string)username, (std::string)password);
                auto token = std::format("{}", hash);
                authedUsers[userid] = token;
                return crow::response(200, token);
            }
            else{
                return crow::response(403, "Invalid Token");
            }
        });
        CROW_ROUTE(app, "/user/register")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x){
                return crow::response(400);
            }
        
            auto username = x["username"].s();
            auto password = x["password"].s();
            auto alreadyRegistered = db->users->get_where("username",username).size() > 0;
            if(username.size() > 0 && password.size() > 0 && !alreadyRegistered){ // we dont want empty usernames and passwords
                auto userid = db->users->add({"username", "password"},{username, password});
                auto userdataid = db->userdata->add({"agegroup","occupation","userId"},{"18-24","school",std::format("{}", userid)});
                db->users->modify(userid, {"userdataId"},{std::format("{}", userdataid)});
                DefaultSettings(userid);
                return crow::response(200);
            }
            else{
                return crow::response(400);
            }
        });
        CROW_ROUTE(app, "/user/data/update")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x){
                return crow::response(400);
            }
            auto token = z["token"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if(authedUsers[userid] == token){
                auto data = z.at("data");
                for(auto i = data.begin(); i != data.end(); ++i){
                    auto key = i.key();
                    auto value = i.value().front().get<std::string>();
                    db->userdata->modify(userid,{key},{value});
                }
                return crow::response(200);
            }
            else{
                return crow::response(403);
            }
        });
        
        CROW_ROUTE(app, "/journals/new")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            nlohmann::json z = nlohmann::json::parse(req.body);
            if (!x){
                return crow::response(400);
            }
            auto token = z["token"].get<std::string>();
            auto comment = z["comment"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if(authedUsers[userid] == token){

                auto data = z.at("data").items();
                auto journalid = db->journals->add({"comment","userId"}, {comment, std::format("{}",userid)});
                for(auto i = data.begin(); i != data.end(); ++i){
                    auto questionId = i.value().back().get<std::string>();
                    auto answer = i.value().front().get<std::string>();
                    db->answers->add({"answer","journalId","questionId"},{answer, std::format("{}",journalid), questionId});
                }
                return crow::response(200);
            }
            else{
                return crow::response(403);
            }
        });
        CROW_ROUTE(app, "/journals/get/<int>") //journal id
        ([&](int id) {
            crow::json::wvalue x({});
            if (id < 0){
                x["error"] = "Invalid id";
                return x;
            }
            x["comment"] = db->journals->get(id)["comment"];
            x["answers"] = db->answers->get_where("journalId", std::format("{}",id));
            return x;
        });
        
        CROW_ROUTE(app, "/journals/ids/<int>") //user id
        ([&](int id) {
            crow::json::wvalue x({});
            if (id < 0){
                x["error"] = "Invalid id";
                return x;
            }
            auto journals = db->journals->get_where("userId",std::format("{}",id));
            x = std::move(journals); //Funky solution to make it only return json array
            return std::move(x); 
        });
        
        CROW_ROUTE(app, "/journals/delete/<int>/<int>/<string>")
        .methods("DELETE"_method)
        ([&](int userid, int journalid, std::string token) {
            if(authedUsers[userid] == token){
                db->journals->delete_item(journalid);
                return crow::response(202);
            }
            else{
                return crow::response(403);
            }
        });
        
        CROW_ROUTE(app, "/settings/get/<int>")
        ([&](int userid) {
            std::vector<crow::json::wvalue> vec;
            crow::json::wvalue x({});
            if (userid < 0){
                x["error"] = "Invalid id";
                return x;
            }
            auto settings = db->settings->get_where("userId", std::format("{}",userid));
            for(auto setting : settings){
                crow::json::wvalue z({});
                z["key"] = db->settings->get(setting)["key"];
                z["value"] = db->settings->get(setting)["value"];
                z["id"] = setting;
                vec.push_back(z);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/settings/update")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x){
                return crow::response(400);
            }
            auto token = z["token"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if(authedUsers[userid] == token){
                auto data = z.at("settings");
                auto userSettings = db->settings->get_where("userId",std::format("{}",userid));
                for(auto i = data.begin(); i != data.end(); ++i){
                    auto key = i.key();
                    auto value = i.value().front().get<std::string>();
                    for(auto setting : userSettings){
                        auto settingsRow = db->settings->get(setting);
                        if(settingsRow["key"] == key){
                            db->settings->modify(setting,{"value"},{value});
                        }
                    }
                }
                return crow::response(200);
            }
            else{
                return crow::response(403);
            }
        });
        CROW_ROUTE(app, "/questions/defaults")
        ([&]() {
            auto questions = db->questions->get_where("tags","default");
            std::vector<crow::json::wvalue> vec;
            for(auto i = 0; i > questions.size(); i++){
                auto question = db->questions->get(i);
                crow::json::wvalue x({});
                x["id"] = question["id"];
                x["tags"] = question["tags"];
                x["question"] = question["question"];
                vec.push_back(x);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/questions/get/<string>")
        ([&](std::string tag) {
            auto questions = db->questions->get_where("tags",tag);
            std::vector<crow::json::wvalue> vec;
            for(auto i = 0; i > questions.size(); i++){
                auto question = db->questions->get(i);
                crow::json::wvalue x({});
                x["id"] = question["id"];
                x["tags"] = question["tags"];
                x["question"] = question["question"];
                vec.push_back(x);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });

        app.port(port).multithreaded().run();
    }


   private:
    //Called whenever a user is registered to prevent empty settings
    void DefaultSettings(int userId){
        db->settings->add({"key","value","userId"},{"key","value",std::format("{}",userId)});
        db->settings->add({"key","value","userId"},{"key1","value2",std::format("{}",userId)});
        //add more settings here
    }
    unsigned long Hash(std::string username, std::string password){
        auto hash1 = std::hash<std::string>{}(username);
        auto hash2 = std::hash<std::string>{}(password);
        auto combinedhash = hash1 ^ (hash2 << 1);
        return combinedhash;
    }
    int UserIdFromToken(std::string token){
        for(auto user : authedUsers){
            if (user.second == token){
                return user.first;
            }
        }
        return 0;
    }
};