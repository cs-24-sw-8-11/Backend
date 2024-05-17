#include <format>
#include <chrono>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>

#include <nlohmann/json.hpp>

#include "Database.hpp"
#include "TestTemplate.hpp"

nlohmann::json config;

auto num_additions = 100;

class DbTest : public Test<std::function<void()>> {
    void init() {
        std::remove("/tmp/db.db3");
        db = Database("/tmp/db.db3");
        std::ifstream file("files/testdata/default.json");
        config = nlohmann::json::parse(file);
        default_username = config["user"]["username"].get<std::string>();
        default_password = config["user"]["password"].get<std::string>();
        default_questions = config["questions"].get<std::vector<std::string>>();
        now = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();

        // add temporary data
        db["users"].add({
            {"username", default_username},
            {"password", default_password},
            {"state", to_string(TRAINING)}
        });
        user_id = db["users"].get_where("username", default_username)[0];
        db["journals"].add({
            {"userId", to_string(user_id)},
            {"timestamp", to_string(now)}
        });
        journal_id = db["journals"].get_where("userId", user_id)[0];
        for (auto question : default_questions)
            db["questions"].add({
                {"question", question},
                {"tags", "default"},
                {"type", to_string(BOOLEAN)}
            });
        question_id = db["questions"].get_where()[0];
    }
    public:
        Database db;
        std::string default_username;
        std::string default_password;
        std::vector<std::string> default_questions;
        int user_id;
        int journal_id;
        int question_id;
        int now;
};

int main() {
    DbTest users;
    users.add_test("add-user", [&](){
        for (auto i = 0; i < num_additions; i++) {
            users.db["users"].add({
                {"username", format("user{}", i)},
                {"password", users.default_password},
                {"state", to_string(TRAINING)}
            });
        }
        // +1 to consider temp data
        assert(users.db["users"].size() == num_additions+1);
    });

    users.add_test("delete-user", [&](){
        assert(users.db["users"].size() == 1);
        users.db["users"].delete_item(users.user_id);
        assert(users.db["users"].size() == 0);
    });

    users.add_test("modify-user", [&](){
        auto current_data = users.db["users"].get(users.user_id);
        assert(current_data == users.db["users"].get(users.user_id));
        users.db["users"].modify(users.user_id, {"username"}, {"tester1"});
        auto new_data = users.db["users"].get(users.user_id);
        assert(current_data != new_data);
    });

    DbTest journals;
    journals.add_test("add-journal", [&](){
        for (auto i = 0; i < num_additions; i++) {
            journals.db["journals"].add({
                {"userId", to_string(journals.user_id)},
                {"timestamp", to_string(journals.now)}
            });
        }
        assert(journals.db["journals"].size() == num_additions+1);
    });
    journals.add_test("delete-journal", [&](){
        assert(journals.db["journals"].size() == 1);
        auto journal_id = journals.db["journals"].get_where()[0];
        journals.db["journals"].delete_item(journal_id);
        assert(journals.db["journals"].size() == 0);
    });
    journals.add_test("modify-journal", [&](){
        journals.db["journals"].add({
            {"userId", to_string(journals.user_id)},
            {"timestamp", to_string(journals.now)}
        });
        auto journal_id = journals.db["journals"].get_where()[0];
        auto current_data = journals.db["journals"].get(journal_id);
        assert(current_data == journals.db["journals"].get(journal_id));
        journals.db["journals"].modify(
            journal_id,
            {"userId"},
            {to_string(journals.user_id+1)});
        auto new_data = journals.db["journals"].get(journal_id);
        assert(current_data != new_data);
    });

    DbTest questions;
    questions.add_test("add-question", [&](){
        assert(questions.db["questions"].size() ==
               questions.default_questions.size());
    });

    questions.add_test("delete-question", [&](){
        assert(questions.db["questions"].size() ==
               questions.default_questions.size());

        auto ids = questions.db["questions"].get_where();
        for (auto id : ids) {
            questions.db["questions"].delete_item(id);
        }
        assert(questions.db["questions"].size() == 0);
    });
    questions.add_test("modify-question", [&](){
        auto current_data = questions.db["questions"].get(questions.question_id);
        assert(current_data ==
               questions.db["questions"].get(questions.question_id));
        questions.db["questions"].modify(questions.question_id,
            {"tags"},
            {"notdefault"});
        auto new_data = questions.db["questions"].get(questions.question_id);
        assert(current_data != new_data);
    });
    DbTest answers;
    answers.add_test("add-answer", [&](){
        for (auto i = 0; i < num_additions; i++) {
            answers.db["answers"].add({
                {"value", to_string(i)},
                {"rating", "4"},
                {"journalId", to_string(answers.journal_id)},
                {"questionId", to_string(answers.question_id)}
            });
        }
        assert(answers.db["answers"].size() == num_additions);
    });

    answers.add_test("delete-answer", [&](){
        answers.db["answers"].add({
            {"value", "3"},
            {"rating", "4"},
            {"journalId", to_string(answers.journal_id)},
            {"questionId", to_string(answers.question_id)}
        });
        assert(answers.db["answers"].size() == 1);
        auto answer_id = answers.db["answers"].get_where()[0];

        answers.db["answers"].delete_item(answer_id);
        assert(answers.db["answers"].size() == 0);
    });
    answers.add_test("modify-answer", [&](){
        answers.db["answers"].add({
            {"value", "4"},
            {"rating", "3"},
            {"journalId", to_string(answers.journal_id)},
            {"questionId", to_string(answers.question_id)}
        });
        auto answer_id = answers.db["answers"].get_where()[0];
        auto current_data = answers.db["answers"].get(answer_id);
        assert(current_data == answers.db["answers"].get(answer_id));
        answers.db["answers"].modify(answer_id, {"value"}, {"5"});
        auto new_data = answers.db["answers"].get(answer_id);
        assert(current_data != new_data);
    });

    DbTest settings;
    settings.add_test("add-setting", [&](){
        for (auto i = 0; i < num_additions; i++) {
            settings.db["settings"].add({
                {"userId", to_string(settings.user_id)},
                {"key", format("setting {}", i)},
                {"value", format("value {}", i)}
            });
        }
        assert(settings.db["settings"].size() == num_additions);
    });
    settings.add_test("delete-setting", [&](){
        settings.db["settings"].add({
            {"userId", to_string(settings.user_id)},
            {"key", "key"},
            {"value", "value"}
        });
        assert(settings.db["settings"].size() == 1);
        auto setting_id = settings.db["settings"].get_where()[0];
        settings.db["settings"].delete_item(setting_id);
        assert(settings.db["settings"].size() == 0);
    });
    settings.add_test("modify-setting", [&](){
        settings.db["settings"].add({
            {"userId", to_string(settings.user_id)},
            {"key", "key"},
            {"value", "value"}
        });
        auto setting_id = settings.db["settings"].get_where()[0];
        auto current_data = settings.db["settings"].get(setting_id);
        assert(current_data == settings.db["settings"].get(setting_id));
        settings.db["settings"].modify(setting_id, {"value"}, {"bettervalue"});
        auto new_data = settings.db["settings"].get(setting_id);
        assert(current_data != new_data);
    });

    DbTest userdata;
    userdata.add_test("delete-userdata", [&](){
        userdata.db["userdata"].add({
            {"education", "0"},
            {"urban", "0"},
            {"gender", "0"},
            {"religion", "0"},
            {"orientation", "0"},
            {"race", "0"},
            {"married", "0"},
            {"age", "0"},
            {"pets", "0"},
            {"userId", to_string(userdata.user_id)}
        });
        assert(userdata.db["userdata"].size() == 1);
        auto userdata_id = userdata.db["userdata"].get_where()[0];
        userdata.db["userdata"].delete_item(userdata_id);
        assert(userdata.db["userdata"].size() == 0);
    });
    userdata.add_test("modify-userdata", [&](){
        userdata.db["userdata"].add({
            {"education", "0"},
            {"urban", "0"},
            {"gender", "0"},
            {"religion", "0"},
            {"orientation", "0"},
            {"race", "0"},
            {"married", "0"},
            {"age", "0"},
            {"pets", "0"},
            {"userId", to_string(userdata.user_id)}
        });
        auto userdata_id = userdata.db["userdata"].get_where()[0];
        auto current_data = userdata.db["userdata"].get(userdata_id);
        assert(current_data == userdata.db["userdata"].get(userdata_id));
        userdata.db["userdata"].modify(userdata_id, {"age"}, {"1"});
        auto new_data = userdata.db["userdata"].get(userdata_id);
        assert(current_data != new_data);
    });

    DbTest predictions;
    predictions.add_test("add-prediction", [&](){
        for (auto i = 0; i < num_additions; i++) {
            predictions.db["predictions"].add({
                {"userId", to_string(predictions.user_id)},
                {"value", to_string(i)},
                {"timestamp", "0"}
            });
        }
        assert(predictions.db["predictions"].size() == num_additions);
    });
    predictions.add_test("delete-prediction", [&](){
        predictions.db["predictions"].add({
            {"userId", to_string(predictions.user_id)},
            {"value", "0"},
            {"timestamp", "0"}
        });
        assert(predictions.db["predictions"].size() == 1);
        auto prediction_id = predictions.db["predictions"].get_where()[0];
        predictions.db["predictions"].delete_item(prediction_id);
        assert(predictions.db["predictions"].size() == 0);
    });
    predictions.add_test("modify-prediction", [&](){
        predictions.db["predictions"].add({
            {"userId", to_string(predictions.user_id)},
            {"value", "0"},
            {"timestamp", "0"}
        });
        auto prediction_id = predictions.db["predictions"].get_where()[0];
        auto current_data = predictions.db["predictions"].get(prediction_id);
        assert(current_data == predictions.db["predictions"].get(prediction_id));
        predictions.db["predictions"].modify(prediction_id, {"value"}, {"1"});
        auto new_data = predictions.db["predictions"].get(prediction_id);
        assert(current_data != new_data);
    });

    // run tests
    users.run();
    journals.run();
    questions.run();
    answers.run();
    settings.run();
    userdata.run();
    predictions.run();
}
