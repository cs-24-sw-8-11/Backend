add_executable(backend backend.cpp)
add_executable(database_test database_test.cpp)
add_executable(predictions_test predictions_test.cpp)
add_executable(utils_test utils_test.cpp)

target_link_libraries(database_test
    SQLiteCpp
    sqlite3
    nlohmann_json
)

target_link_libraries(predictions_test
    nlohmann_json
    SQLiteCpp
)
target_link_libraries(backend
    SQLiteCpp
    sqlite3
    argparse
    httplib
    nlohmann_json
)
target_link_libraries(utils_test
    nlohmann_json
    SQLiteCpp
)

add_test(NAME database_test COMMAND database_test WORKING_DIRECTORY ..)
add_test(NAME predictions_test COMMAND predictions_test WORKING_DIRECTORY ..)
add_test(NAME utils_test COMMAND utils_test WORKING_DIRECTORY ..)
