#include <iostream>
#include <argparse/argparse.hpp>

#include "Api.hpp"
#include "Globals.hpp"

/// @brief Adds default questions and mitigations to the database
/// @param path db path
void default_setup(std::string path) {
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
            "mitigation"}, {
            "1",
            "default",
            "Default Mitigation so tests don't fail"});
    }
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

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "argparse pooped itself, exiting..." << std::endl;
        exit(1);
    }
    if (program["--verbose"] == true) {
        std::cout << "Verbosity enabled" << std::endl;
        VERBOSE = true;
    }

    auto path = program.get<std::string>("--database");
    auto port = program.get<int>("--port");
    setup(path);
    Api api(path, port);
}
