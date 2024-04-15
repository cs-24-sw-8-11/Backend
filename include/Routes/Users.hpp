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
        this->server->Get("/user/get/:uid", [&](Request request, Response response){
            json response_data;
            auto uid = stoi(request.path_params.at("uid"));
            auto user = db->users->get(uid);
            auto userdata = db->userdata->get(db->userdata->get_where(
                "userId",
                db_int(uid)).front());
            if (uid < 0) {
                response_data["error"] = "Invalid id";
                response.set_content(response_data, "application/json");
                return;
            }
            for (auto key : user.keys()) {
                if (key == "password") {
                    continue;
                }
                response_data[key] = user[key];
            }
            for (auto key : userdata.keys()) {
                response_data[key] = userdata[key];
            }
            response.set_content(response_data, "application/json");
        });
        this->server->Get("/user/get/:uid", [&](Request request, Response response){
            json response_data;
            auto uid = stoi(request.path_params.at("uid"));
            auto user = db->users->get(uid);
            auto userdata = db->userdata->get(db->userdata->get_where(
                "userId",
                db_int(uid)).front());
            if (uid < 0) {
                response_data["error"] = "Invalid id";
                response.set_content(response_data, "application/json");
            }
            for (auto key : user.keys()) {
                if (key == "password") {
                    continue;
                }
                response_data[key] = user[key];
            }
            for (auto key : userdata.keys()) {
                response_data[key] = userdata[key];
            }
            response.set_content(response_data, "application/json");

        });
        this->server->Get("/user/ids/:min-:max", [&](Request request, Response response){
            json response_data;
            auto min = stoi(request.path_params.at("min"));
            auto max = stoi(request.path_params.at("max"));
            if (min < 0 || max < 0 || (max-min < 0)) {
                response_data["difference"] = max-min;
                response_data["max"] = max;
                response_data["min"] = min;
                response_data["error"] = "Invalid Range";
                return response.set_content(response_data, "application/json");
            }
            auto users = db->users->get_where();
            vector<int> filteredUsers;
            for (const auto & user : users) {
                if (user <= max && user >= min) {
                    filteredUsers.push_back(user);
                }
            }
            // Funky solution to make it only return json array
            response_data = move(filteredUsers);
            // This is done due to "inconsistent type" errors appearing
            // if not done
            response.set_content(move(response_data), "application/json");
        });
        this->server->Get("/user/ids", [&](Request request, Response& response){
            json data = db->users->get_where();
            string result = to_string(data);
            return response.set_content(result, "application/json");
        });
        /*this->app->route_dynamic("/user/ids")
        ([&]() {
            crow::json::wvalue x;
            auto users = db->users->get_where();
            x = std::move(users);
            return std::move(x);
        });
        this->app->route_dynamic("/user/auth")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x) {
                return crow::response(400, "Unable to load/parse json");
            }
            auto username = x["username"].s();
            auto password = x["password"].s();
            if (db->users->get_where("username", username).size() == 0) {
                return crow::response(403, "Invalid Credentials!");
            }
            auto dbpassword = db->users->get(db->users->get_where(
                "username",
                username).front())["password"];

            if (dbpassword == password) {
                auto userid = db->users->get_where(
                    "username",
                    username).front();
                auto hash = make_hash((std::string)username, (std::string)password);
                auto token = std::format("{}", hash);
                authedUsers[userid] = token;
                return crow::response(200, token);
            } else {
                return crow::response(403, "Invalid Credentials!");
            }
        });
        this->app->route_dynamic("/user/register")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x) {
                return crow::response(400, "Unable to load/parse JSON.");
            }
            auto username = x["username"].s();
            auto password = x["password"].s();
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
                return crow::response(200, "Successfully registered!");
            } else {
                return crow::response(400, "Username is already taken!");
            }
        });
        this->app->route_dynamic("/user/data/update")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x) {
                return crow::response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto data = z.at("data");
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto key = i.key();
                    auto value = i.value().front().get<std::string>();
                    db->userdata->modify(userid, {key}, {value});
                }
                return crow::response(200, "Successfully updated user data.");
            } else {
                return crow::response(403,
                    "Token does not match expected value!");
            }
        });*/
    }
};