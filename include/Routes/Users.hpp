#include "Route.hpp"

#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

int64_t make_hash(std::string username, std::string password) {
    auto hash1 = std::hash<std::string>{}(username);
    auto hash2 = std::hash<std::string>{}(password);
    auto combinedhash = hash1 ^ (hash2 << 1);
    return combinedhash;
}

class Users : public Route {
 public:
    using Route::Route;

    virtual void init() override {
        this->server->Get("/user/get/:uid", [&](Request request, Response& response){
            json response_data;
            auto uid = stoi(request.path_params["uid"]);
            auto user = db->users->get(uid);
            auto userdata = db->userdata->get(db->userdata->get_where(
                "userId",
                db_int(uid)).front());
            if (uid < 0) {
                response_data["error"] = "Invalid Id.";
                respond(response, response_data, 400);
                return;
            }
            for (auto key : user.keys()) {
                if (key == "password") 
                    continue;
                
                response_data[key] = user[key];
            }
            for (auto key : userdata.keys()) 
                response_data[key] = userdata[key];
            
            respond(response, response_data);
        });
        this->server->Get("/user/ids/:min/:max", [&](Request request, Response& response){
            json response_data;
            auto min = stoi(request.path_params["min"]);
            auto max = stoi(request.path_params["max"]);
            cout << "test" << min << " " << max << endl;
            if (min < 0 || max < 0 || (max-min < 0)) {
                response_data["difference"] = max-min;
                response_data["max"] = max;
                response_data["min"] = min;
                response_data["error"] = "Invalid Range.";
                return respond(response, response_data);
            }
            auto users = db->users->get_where();
            vector<int> filteredUsers;
            for (const auto & user : users) {
                if (user <= max && user >= min) 
                    filteredUsers.push_back(user);
                
            }
            response_data = filteredUsers;
            respond(response, response_data);
        });
        this->server->Get("/user/ids", [&](Request request, Response& response){
            json data = db->users->get_where();
            respond(response, data);
        });
        this->server->Post("/user/auth", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto username = body["username"].get<string>();
            auto password = body["password"].get<string>();
            if (db->users->get_where("username", username).size() == 0) {
                return respond(response, string("Invalid Credentials!"), 403);
            }
            auto dbpassword = db->users->get(db->users->get_where(
                "username",
                username).front())["password"];

            if (dbpassword == password) {
                auto userid = db->users->get_where(
                    "username",
                    username).front();
                auto hash = make_hash((string)username, (string)password);
                auto token = std::format("{}", hash);
                authedUsers[userid] = token;
                respond(response, token);
            } else 
                respond(response, string("Invalid Credentials!"), 403);
            

        });
        this->server->Post("/user/register", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto username = body["username"].get<string>();
            auto password = body["password"].get<string>();
            auto alreadyRegistered = db->users->get_where(
                "username",
                username).size() > 0;
            // we dont want empty usernames and passwords
            if (username.size() > 0 &&
                password.size() > 0 &&
                !alreadyRegistered) {
                auto userid = db->users->add({
                    "username",
                    "password",
                    "state"}, {
                    username,
                    password,
                    db_int(TRAINING)});
                auto userdataid = db->userdata->add({
                    "agegroup",
                    "occupation",
                    "userId"}, {
                    "18-24",
                    "school",
                    db_int(userid)});
                db->users->modify(userid, {"userdataId"}, {db_int(userdataid)});
                DefaultSettings(userid);
                respond(response, string("Successfully registered!"));
            } else 
                respond(response, string("Username is already taken!"), 400);
            
        });
        this->server->Post("/user/data/update", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<std::string>();
            auto uid = UserIdFromToken(token);
            if (authedUsers[uid] == token) {
                auto data = body["data"];
                for(auto [key, value] : data.items()){
                    db->userdata->modify(uid, {key}, {value});
                }
                respond(response, string("Successfully updated user data."));
            } else {
                respond(response, string("Token does not match expected value!"), 403);
            }

        });
    }
};