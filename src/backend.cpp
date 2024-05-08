#include <ranges>
#include <future>
#include <iostream>
#include <argparse/argparse.hpp>

#include "Api.hpp"
#include "Globals.hpp"
#include "PredictionManager.hpp"
#include "Utils.hpp"

using namespace std;
using namespace std::views;

/// @brief Adds default questions and mitigations to the database
/// @param path db path
void setup(std::string path) {
    auto db = std::make_shared<Database>(path);
    if (db->questions->get_where("tags", "default").size() == 0) {
        db->questions->add({"type",
            "tags",
            "question"}, {
            "1",
            "default",
            "Does this default question stop the tests from failing?"});
    }
    if (db->mitigations->get_where("tags", "default").size() == 0) {
        db->mitigations->add({"type",
            "tags",
            "title",
            "description"}, {
            "1",
            "default",
            "Default Mitigation so tests don't fail",
            "Default description because it cannot be null."});
    }
}

enum Mode{
    DEFAULT,
    CHECK,
    POPULATE
};
Mode to_mode(string input) {
    if (P8::to_lower_case(input) == "populate")
        return POPULATE;
    else if (P8::to_lower_case(input) == "check")
        return CHECK;
    else
        return DEFAULT;
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
    program.add_argument("--dataset", "-d")
        .help("Path to the dataset for populating the database")
        .default_value("./lib/datasets")
        .nargs(1);
    program.add_argument("--threads", "-t")
        .help("Specify number of threads that can be used")
        .scan<'d', int>()
        .default_value(4)
        .nargs(1);
    program.add_argument("--tables")
        .help("For the populate command, specify which tables to populate")
        .default_value("all")
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
    switch (to_mode(program.get<string>("mode"))) {
        case DEFAULT: {
            auto path = program.get<string>("--database");
            auto port = program.get<int>("--port");
            if (port < 1) {
                cerr << "Error, port number can't be less than 1" << endl;
                exit(1);
            }
            setup(path);
            Api api(path, port);
            break;
        }
        case CHECK: {
            cout << "Checking database contents..." << endl;
            auto path = program.get<string>("--database");
            auto thread_cnt = program.get<int>("--threads");
            if (thread_cnt < 1) {
                cerr << "Error, thread count can't be less than 1" << endl;
                exit(1);
            }
            Database db{path};
            auto uids = db.users->get_where();
            auto uids_size = uids.size();
            P8::ThreadPool<pair<int, int>, double> pool{thread_cnt};
            vector<pair<int, int>> args;
            for (auto [i, uid] : zip(P8::make_range(uids_size), uids)) {
                args.push_back(make_pair(i, uid));
            }
            auto target = [=](pair<int, int> arg) {
                auto i = arg.first;
                auto uid = arg.second;
                auto jids = db.journals->get_where("userId", uid);
                vector<double> results;
                PredictionManager manager;
                auto builder = manager.create_new_prediction(uid);
                for(auto [day_identifier, jid] : views::zip(P8::make_range(3), jids)){
                    auto journal = db.journals->get(jid);
                    vector<pair<double, double>> prediction_data;
                    for(auto aid : db.answers->get_where("journalId", jid)){
                        auto answer = db.answers->get(aid);
                        prediction_data.push_back(make_pair(stod(answer["value"]), stod(answer["rating"])));
                    }
                    builder.add_journal(day_identifier, prediction_data);
                }
                auto value = builder.build();
                return value;
            };
            auto logger = [](pair<int, int> in, double out){
                auto i = in.first;
                auto uid = in.second;
                cout << "i: " << i << " uid: " << uid << " result: " << out << endl;
            };

            auto results = pool.map(target, args, logger);
            for (auto result : results)
                cout << result << endl;
            break;
        }
        case POPULATE: {
            auto prefix = program.get<string>("--dataset");
            auto database = program.get<string>("--database");
            auto tables = program.get<string>("--tables");
            cout << "REMOVING DATABASE IN 10 SECONDS" << endl;
            for (auto i = 10; i >= 0; i--) {
                sleep(1);
                cout << i << endl;
            }
            auto rm_result = system(format("rm {}", database).c_str());
            cout << "Creating database" << endl;
            {
                auto db = Database{database};
            }
            auto result = system(format("python {0}/dataset_to_db.py -f {0}/data.csv -c {0}/codebook.txt -m {0}/mitigations.csv -t {1}", prefix, tables).c_str());
            cout << "Was database removed?:        " << rm_result << endl;
            cout << "Populate command return code: " << result << endl;
            break;
        }
    }
}
