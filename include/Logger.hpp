#pragma once

#include <ostream>
#include <fstream>
#include <string>

#include "Globals.hpp"

using namespace std;

namespace P8 {
    auto verbosity = 0;

    enum Verbosity {
        ALL,
        IMPORTANT,
        DEBUG,
        INFO
    };

    void writefile(string input) {
        ofstream log_stream{logfile};
        log_stream << input;
        log_stream.close();
    }

    template<Verbosity V = ALL, typename... Args>
    constexpr void log(format_string<Args...> fmt, Args&&... args) {
        auto input = format(fmt , forward<Args>(args)...);
        writefile(input);

        if (verbosity >= V) {
            cout << input << endl;
        }
    }

    template<Verbosity V = ALL, typename... Args>
    constexpr void loge(format_string<Args...> fmt, Args&&... args){
        auto input = format(fmt, forward<Args>(args)...);
        writefile(input);

        if (verbosity >= V) {
            cerr << "\033[38;2;255;0;0m" <<input << "\033[0m" << endl;
        }
    }

    template<Verbosity V = ALL, typename... Args>
    constexpr void logw(format_string<Args...> fmt, Args&&... args){
        auto input = format(fmt, forward<Args>(args)...);
        writefile(input);

        if (verbosity >= V) {
            cout << "\033[38;2;255;255;0m" << input << "\033[0m" << endl;
        }
    }
}  // namespace P8
