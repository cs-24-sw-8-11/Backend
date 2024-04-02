#include <crow.h>

#include <iostream>
#include <string>
#include "Database.hpp"

class API {
   public:
    crow::SimpleApp app;
    std::shared_ptr<Database> db;
    API(std::string path) {
        db = std::make_shared<Database>(path);

        CROW_ROUTE(app, "/")
        ([]() {
            return "Hello world";
        });
        CROW_ROUTE(app, "/json")
        ([] {
            crow::json::wvalue x({{"message", "Hello, World!"}});
            x["message2"] = "Hello, World.. Again!";
            return x;
        });
        CROW_ROUTE(app, "/hello/<int>")
        ([](int count) {
            if (count > 100)
                return crow::response(400);
            std::ostringstream os;
            os << count << " bottles of beer!";
            return crow::response(os.str());
        });
        CROW_ROUTE(app, "/stress")
        ([] {
            std::string s = "Stress Level: 0";
            for (int i = 0; i < 1000; i++)
                s += std::format("\nStress Level: {0}", i + 1);
            return s;
        });
        CROW_ROUTE(app, "/db")([&]{
            //std::string s = "DB Test:\n";
            auto users = db->users;
            auto size = users->get_where("username","taoshi");
            crow::json::wvalue x({});
            x["username"] = "thomas";
            x["password"] = "thomas";
            return x;
            //s += std::format("{}", size);
            //return crow::response(s);
        });

        app.port(8080).multithreaded().run();
    }

   private:
};