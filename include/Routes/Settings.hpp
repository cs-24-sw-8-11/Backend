#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace httplib;
using namespace nlohmann;

class Settings : public Route {
    using Route::Route;
 public:
    virtual void init() override {
        this->server->Get("/settings/get/:uid", [&](Request request, Response response){
            auto uid = stoi(request.path_params.at("uid"));
            vector<json> vec;
            json response_data;
            if (uid < 0) {
                response_data["error"] = "Invalid id";
                return response.set_content(response_data, "application/json");
            }
            auto settings = db->settings->get_where(
                "userId",
                db_int(uid));
            for (auto setting : settings) {
                json data({});
                auto row = db-> settings->get(setting);
                for (auto key : row.keys()) {
                    data[key] = row[key];
                }
                vec.push_back(data);
            }
            response_data = move(vec);
            response.set_content(move(response_data), "application/json");
        });
        this->server->Post("/settings/update", [&](Request request, Response response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<std::string>();
            auto uid = UserIdFromToken(token);
            if (authedUsers[uid] == token) {
                auto data = body.at("settings");
                auto userSettings = db->settings->get_where(
                    "userId",
                    db_int(uid));
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto key = i.key();
                    auto value = i.value().front().get<std::string>();
                    if (!SettingExists(userSettings, key)) {
                        db->settings->add({
                            "key",
                            "value",
                            "userId"}, {
                            key,
                            value,
                            db_int(uid)});
                    }
                    for (auto setting : userSettings) {
                        auto settingsRow = db->settings->get(setting);
                        if (settingsRow["key"] == key) {
                            db->settings->modify(setting, {"value"}, {value});
                        }
                    }
                }
                response.status = 200;
                response.set_content("Successfully updated settings", "text/plain");
            } else{
                response.status = 403;
                response.set_content("Token does not match expected value!", "text/plain");
            }
        });
    }
};