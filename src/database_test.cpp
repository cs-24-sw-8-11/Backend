#include <format>
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

using namespace std;
using namespace nlohmann;

class DBTest : public Test<function<void()>> {
    void init() {
        std::remove("/tmp/db.db3");
        db = std::make_shared<Database>("/tmp/db.db3");
        ifstream file("files/testdata/default.json");
        config = json::parse(file);
        default_username = config["user"]["username"].get<string>();
        default_password = config["user"]["password"].get<string>();
        default_questions = config["questions"].get<vector<string>>();

        // add temporary data
        db->users->add({"username",
            "password",
            "state"}, {
            default_username,
            default_password,
            db_int(TRAINING)});
        user_id = db->users->get_where("username", default_username)[0];
        db->journals->add({"userId"}, {db_int(user_id)});
        journal_id = db->journals->get_where("userId", db_int(user_id))[0];
        for (auto question : default_questions)
            db->questions->add({
                "question",
                "tags",
                "type"}, {
                question,
                "default",
                db_int(BOOLEAN)});
        question_id = db->questions->get_where()[0];
    }
    public:
        shared_ptr<Database> db;
        string default_username;
        string default_password;
        vector<string> default_questions;
        int user_id;
        int journal_id;
        int question_id;
};

int main() {
    DBTest users;
    users.add_test("add-user", [&](){
        for (auto i = 0; i < num_additions; i++) {
            users.db->users->add({
                "username",
                "password",
                "state"
            }, {
                format("user{}", i),
                users.default_password,
                db_int(TRAINING)
            });
        }
        // +1 to consider temp data
        assert(users.db->users->size() == num_additions+1);
    });

    users.add_test("delete-user", [&](){
        assert(users.db->users->size() == 1);
        users.db->users->delete_item(users.user_id);
        assert(users.db->users->size() == 0);
    });

    users.add_test("modify-user", [&](){
        auto current_data = users.db->users->get(users.user_id);
        assert(current_data == users.db->users->get(users.user_id));
        users.db->users->modify(users.user_id, {"username"}, {"tester1"});
        auto new_data = users.db->users->get(users.user_id);
        assert(current_data != new_data);
    });

    DBTest journals;
    journals.add_test("add-journal", [&](){
        for (auto i = 0; i < num_additions; i++) {
            journals.db->journals->add({"userId"}, {db_int(journals.user_id)});
        }
        assert(journals.db->journals->size() == num_additions+1);
    });
    journals.add_test("delete-journal", [&](){
        assert(journals.db->journals->size() == 1);
        auto journal_id = journals.db->journals->get_where()[0];
        journals.db->journals->delete_item(journal_id);
        assert(journals.db->journals->size() == 0);
    });
    journals.add_test("modify-journal", [&](){
        journals.db->journals->add({"userId"}, {db_int(journals.user_id)});
        auto journal_id = journals.db->journals->get_where()[0];
        auto current_data = journals.db->journals->get(journal_id);
        assert(current_data == journals.db->journals->get(journal_id));
        journals.db->journals->modify(
            journal_id,
            {"userId"},
            {db_int(journals.user_id+1)});
        auto new_data = journals.db->journals->get(journal_id);
        assert(current_data != new_data);
    });

    DBTest questions;
    questions.add_test("add-question", [&](){
        assert(questions.db->questions->size() ==
               questions.default_questions.size());
    });

    questions.add_test("delete-question", [&](){
        assert(questions.db->questions->size() ==
               questions.default_questions.size());

        auto ids = questions.db->questions->get_where();
        for (auto id : ids) {
            questions.db->questions->delete_item(id);
        }
        assert(questions.db->questions->size() == 0);
    });
    questions.add_test("modify-question", [&](){
        auto current_data = questions.db->questions->get(questions.question_id);
        assert(current_data ==
               questions.db->questions->get(questions.question_id));
        questions.db->questions->modify(questions.question_id,
            {"tags"},
            {"notdefault"});
        auto new_data = questions.db->questions->get(questions.question_id);
        assert(current_data != new_data);
    });
    DBTest answers;
    answers.add_test("add-answer", [&](){
        for (auto i = 0; i < num_additions; i++) {
            answers.db->answers->add({
                "answer",
                "journalId",
                "questionId"}, {
                format("answer{}", i),
                db_int(answers.journal_id),
                db_int(answers.question_id)});
        }
        assert(answers.db->answers->size() == num_additions);
    });

    answers.add_test("delete-answer", [&](){
        answers.db->answers->add({
            "answer",
            "journalId",
            "questionId"}, {
            "answer",
            db_int(answers.journal_id),
            db_int(answers.question_id)});
        assert(answers.db->answers->size() == 1);
        auto answer_id = answers.db->answers->get_where()[0];

        answers.db->answers->delete_item(answer_id);
        assert(answers.db->answers->size() == 0);
    });
    answers.add_test("modify-answer", [&](){
        answers.db->answers->add({
            "answer",
            "journalId",
            "questionId"}, {
            "answer",
            db_int(answers.journal_id),
            db_int(answers.question_id)});
        auto answer_id = answers.db->answers->get_where()[0];
        auto current_data = answers.db->answers->get(answer_id);
        assert(current_data == answers.db->answers->get(answer_id));
        answers.db->answers->modify(answer_id, {"answer"}, {"betteranswer"});
        auto new_data = answers.db->answers->get(answer_id);
        assert(current_data != new_data);
    });

    DBTest settings;
    settings.add_test("add-setting", [&](){
        for (auto i = 0; i < num_additions; i++) {
            settings.db->settings->add({
                "userId",
                "key",
                "value"}, {
                db_int(settings.user_id),
                format("setting {}", i),
                format("value {}", i)});
        }
        assert(settings.db->settings->size() == num_additions);
    });
    settings.add_test("delete-setting", [&](){
        settings.db->settings->add({
            "userId",
            "key",
            "value"}, {
            db_int(settings.user_id),
            "testkey",
            "testvalue"});
        assert(settings.db->settings->size() == 1);
        auto setting_id = settings.db->settings->get_where()[0];
        settings.db->settings->delete_item(setting_id);
        assert(settings.db->settings->size() == 0);
    });
    settings.add_test("modify-setting", [&](){
        settings.db->settings->add({
            "userId",
            "key",
            "value"}, {
            db_int(settings.user_id),
            "testkey",
            "testvalue"});
        auto setting_id = settings.db->settings->get_where()[0];
        auto current_data = settings.db->settings->get(setting_id);
        assert(current_data == settings.db->settings->get(setting_id));
        settings.db->settings->modify(setting_id, {"value"}, {"bettervalue"});
        auto new_data = settings.db->settings->get(setting_id);
        assert(current_data != new_data);
    });

    DBTest userdata;
    userdata.add_test("delete-userdata", [&](){
        userdata.db->userdata->add({
            "agegroup",
            "occupation",
            "userId"}, {
            "50-54",
            "unemployed",
            db_int(userdata.user_id)});
        assert(userdata.db->userdata->size() == 1);
        auto userdata_id = userdata.db->userdata->get_where()[0];
        userdata.db->userdata->delete_item(userdata_id);
        assert(userdata.db->userdata->size() == 0);
    });
    userdata.add_test("modify-userdata", [&](){
        userdata.db->userdata->add({
            "agegroup",
            "occupation",
            "userId"}, {
            "50-54",
            "unemployed",
            db_int(userdata.user_id)});
        auto userdata_id = userdata.db->userdata->get_where()[0];
        auto current_data = userdata.db->userdata->get(userdata_id);
        assert(current_data == userdata.db->userdata->get(userdata_id));
        userdata.db->userdata->modify(userdata_id, {"agegroup"}, {"55-59"});
        auto new_data = userdata.db->userdata->get(userdata_id);
        assert(current_data != new_data);
    });

    DBTest predictions;
    predictions.add_test("add-prediction", [&](){
        for (auto i = 0; i < num_additions; i++) {
            predictions.db->predictions->add({
                "userId",
                "value"}, {
                db_int(predictions.user_id),
                db_int(i)});
        }
        assert(predictions.db->predictions->size() == num_additions);
    });
    predictions.add_test("delete-prediction", [&](){
        predictions.db->predictions->add({
            "userId",
            "value"}, {
            db_int(predictions.user_id),
            "0"});
        assert(predictions.db->predictions->size() == 1);
        auto prediction_id = predictions.db->predictions->get_where()[0];
        predictions.db->predictions->delete_item(prediction_id);
        assert(predictions.db->predictions->size() == 0);
    });
    predictions.add_test("modify-prediction", [&](){
        predictions.db->predictions->add({
            "userId",
            "value"}, {
            db_int(predictions.user_id),
            "0"});
        auto prediction_id = predictions.db->predictions->get_where()[0];
        auto current_data = predictions.db->predictions->get(prediction_id);
        assert(current_data == predictions.db->predictions->get(prediction_id));
        predictions.db->predictions->modify(prediction_id, {"value"}, {"1"});
        auto new_data = predictions.db->predictions->get(prediction_id);
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
