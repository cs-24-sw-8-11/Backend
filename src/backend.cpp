#include <iostream>

#include "Database.hpp"

int main(int argc, char* argv[]){
    std::cout << "Started backend program..." << std::endl;
    Database db{"/tmp/db.db3"};
}