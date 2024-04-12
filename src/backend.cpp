#include <iostream>
#include <argparse/argparse.hpp>

#include "Api.hpp"

void DefaultQuestion(std::string path) {
    std::shared_ptr<Database> db;
    db = std::make_shared<Database>(path);
    if (db->questions->get_where("tags", "default").size() == 0) {
        db->questions->add({"type",
            "tags",
            "question"}, {
            "1",
            "default",
            "How stressed were you today?"});
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
    }

    auto path = program.get<std::string>("--database");
    auto port = program.get<int>("--port");
    DefaultQuestion(path);
    API api(path);
    api.Run(port);
}
