#include <iostream>
#include <argparse/argparse.hpp>

#include "Database.hpp"

int main(int argc, char* argv[]){
    argparse::ArgumentParser program("backend");
    program.add_argument("-v", "--verbose")
        .help("Increase verbosity")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--database", "-d")
        .help("Specify the path to the SQLite database")
        .default_value("/tmp/db.db3")
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
    Database db(path);
}