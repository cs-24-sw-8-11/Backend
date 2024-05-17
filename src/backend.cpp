#include <ranges>
#include <future>
#include <iostream>
#include <argparse/argparse.hpp>

#include "Api.hpp"
#include "Globals.hpp"
#include "PredictionManager.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

using namespace std;
using namespace std::views;
using namespace P8;

/// @brief Adds default questions and mitigations to the database
/// @param path db path
void setup(std::string path) {
    Database db(path);
    if (db["questions"].get_where("tags", "default").size() == 0) {
        db["questions"].add({
            put("type", "1"),
            put("tags", "default"),
            put("question", "Does this default question stop the tests from failing?")
        });
    }
    log<INFO>("added question");
    if (db["mitigations"].get_where("tags", "default").size() == 0) {
        db["mitigations"].add({
            put("type", "1"),
            put("tags", "default"),
            put("title", "Default mitigation so tests don't fail"),
            put("description", "Default description because it cannot be null.")
        });
    }
    log<INFO>("added mitigation");
}

enum Mode{
    DEFAULT,
    CHECK,
    POPULATE
};
Mode to_mode(string input) {
    log<DEBUG>("converting mode: {}", input);
    if (to_lower_case(input) == "populate")
        return POPULATE;
    else if (to_lower_case(input) == "check")
        return CHECK;
    else
        return DEFAULT;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("backend");
    program.add_argument("-v", "--verbose")
        .help("Increase verbosity")
        .action([&](const auto&){ ++verbosity; })
        .append()
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

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
    program.add_argument("--logfile", "-L")
        .help("Specify path to logfile")
        .default_value("/tmp/p8.log")
        .nargs(1);
    program.add_argument("mode")
        .help("Operating mode for the backend")
        .default_value("default");

    try {
        program.parse_args(argc, argv);
    }
    catch (const exception& e) {
        loge("Argparse died, exiting...");
        exit(1);
    }
    logfile = program.get<string>("--logfile");

    log<DEBUG>("Parsed args");
    log<DEBUG>("Verbosity: {}", verbosity);
    log<INFO>("{}", program.get<string>("mode"));
    switch (to_mode(program.get<string>("mode"))) {
        case DEFAULT: {
            auto path = program.get<string>("--database");
            auto port = program.get<int>("--port");
            if (port < 1) {
                loge("Error, port number can't be less than 1");
                exit(1);
            }
            setup(path);
            Api api(path, port);
            break;
        }
        case CHECK: {
            log<INFO>("Checking db contents");
            cout << "Checking database contents..." << endl;
            auto path = program.get<string>("--database");
            auto thread_cnt = program.get<int>("--threads");
            if (thread_cnt < 1) {
                loge("Error, thread count can't be less than 1");
                exit(1);
            }
            Database db{path};
            auto uids = db["users"].get_where();
            auto uids_size = uids.size();
            P8::ThreadPool<pair<int, int>, double> pool{thread_cnt};
            vector<pair<int, int>> args;
            for (auto [i, uid] : zip(make_range(uids_size), uids)) {
                args.push_back(make_pair(i, uid));
            }
            auto target = [&](pair<int, int> arg) {
                auto i = arg.first;
                auto uid = arg.second;
                auto jids = db["journals"].get_where("userId", uid);
                vector<double> results;
                PredictionManager manager;
                auto builder = manager.create_new_prediction(uid);
                for (auto [day_identifier, jid] : views::zip(make_range(3), jids)) {
                    auto journal = db["journals"].get(jid);
                    vector<pair<double, double>> prediction_data;
                    for (auto aid : db["answers"].get_where("journalId", jid)) {
                        auto answer = db["answers"].get(aid);
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
                log("i: {} uid: {} result: {}", i, uid, out);
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
            logw("REMOVING DATABASE IN 10 SECONDS");
            for (auto i = 10; i >= 0; i--) {
                sleep(1);
                logw<IMPORTANT>("{}", i);
            }
            auto rm_result = system(format("rm {}", database).c_str());
            log<DEBUG>("Creating database");
            {
                auto db = Database{database};
            }
            auto result = system(format("python {0}/dataset_to_db.py -f {0}/data.csv -c {0}/codebook.txt -m {0}/mitigations.csv -t {1}", prefix, tables).c_str());
            log<INFO>("Was database removed?:        {}", rm_result);
            log<INFO>("Populate command return code: {}", result);
            break;
        }
    }
}
