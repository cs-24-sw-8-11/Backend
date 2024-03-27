add_executable(backend backend.cpp)
add_executable(database_test database_test.cpp)
add_executable(api_test api_test.cpp)
add_executable(predictions_test predictions_test.cpp)
add_executable(questionaire_test questionaire_test.cpp)

target_link_libraries(database_test
    SQLiteCpp
    sqlite3
)
target_link_libraries(backend
    SQLiteCpp
    sqlite3
)

add_test(NAME database_test COMMAND database_test)
add_test(NAME api_test COMMAND api_test)
add_test(NAME predictions_test COMMAND predictions_test)
add_test(NAME questionaire_test COMMAND questionaire_test)