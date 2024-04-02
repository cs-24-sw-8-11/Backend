#include <iostream>
#include <argparse/argparse.hpp>

#include "Api.hpp"

int main(int argc, char* argv[]){
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
        .default_value(8080)
        .nargs(1);

    try{
        program.parse_args(argc, argv);
    }
    catch(const std::exception& e){
        std::cerr << "argparse pooped itself, exiting..." << std::endl;
        exit(1);
    }
    if(program["--verbose"] == true){
        std::cout << "Verbosity enabled" << std::endl;
    }

    auto path = program.get<std::string>("--database");
    auto port = program.get<int>("--port");
    API api(path, port);
}
