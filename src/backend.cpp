#include <iostream>
#include <argparse/argparse.hpp>
#include <ranges>
#include <future>

#include "Api.hpp"
#include "Globals.hpp"
#include "PredictionManager.hpp"
#include "Utils.hpp"

using namespace std;
using namespace std::ranges;

void default_question(string path) {
    auto db = make_shared<Database>(path);
    if (db->questions->get_where("tags", "default").size() == 0) {
        db->questions->add({"type",
            "tags",
            "question"}, {
            "1",
            "default",
            "How stressed were you today?"});
    }
}

enum Mode{
    DEFAULT,
    CHECK
};
Mode to_mode(string input){
    if (input == "default")
        return DEFAULT;
    else
        return CHECK;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("backend");
    program.add_argument("-v", "--verbose")
        .help("Increase verbosity")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--database", "-d")
        .help("Specify the path to the SQLite database")
        .default_value("db.db3")
        .nargs(1);
    program.add_argument("--port", "-p")
        .help("Specify the port for the API")
        .scan<'d', int>()
        .default_value(8080)
        .nargs(1);
    program.add_argument("mode")
        .help("Operating mode for the backend")
        .default_value("default");

    try {
        program.parse_args(argc, argv);
    }
    catch (const exception& e) {
        cerr << "argparse pooped itself, exiting..." << endl;
        exit(1);
    }
    if (program["--verbose"] == true) {
        cout << "Verbosity enabled" << endl;
        VERBOSE = true;
    }

    switch(to_mode(program.get<string>("mode"))){
        case DEFAULT: {

            auto path = program.get<string>("--database");
            auto port = program.get<int>("--port");
            default_question(path);
            Api api(path, port);
            break;
        }
        case CHECK: {
            cout << "Checking database contents..." << endl;
            auto path = program.get<string>("--database");
            Database db{path};
            auto uids = db.users->get_where();
            auto uids_size = uids.size();
            vector<future<string>> tasks;
            P8::ThreadPool<pair<int, int>, string> pool{4};
            vector<pair<int, int>> args;
            for(auto [i, uid] : zip_view(P8::make_range(uids_size), uids)){
                args.push_back(make_pair(i, uid));
            }
            auto results = pool.map([=](pair<int, int> arg) -> string {
                auto i = arg.first;
                auto uid = arg.second;
                string result_str = "";
                result_str += "["+to_string((i*100)/uids_size)+"%] uid: "+to_string(uid) + " jids: [";
                auto jids = db.journals->get_where("userId", to_string(uid));
                vector<double> results;
                for(auto jid : jids){
                    result_str += to_string(jid) + ",";
                    auto aids = db.answers->get_where("journalId", to_string(jid));
                    PredictionManager manager;
                    auto builder = manager.create_new_prediction(uid);
                    for(auto aid : aids){
                        auto data = db.answers->get(aid);
                        auto qid = data["questionId"];
                        builder.add_valued_data(qid, stod(data["answer"]));
                    }
                    results.push_back(builder.build());
                }
                result_str += "] values: [";
                for(auto result : results)
                    result_str += to_string(result) + ",";
                auto f = calculate_regression(P8::make_range<double>(results.size()), results);
                result_str += "] regression: " + to_string(f(results.size()+1));
                return result_str;
                
            }, args);
            for(auto result : results)
                cout << result << endl;
            break;
        }
    }
}
