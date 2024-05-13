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

    template<Verbosity V = ALL>
    void log(string input) {
        writefile(input);

        if (verbosity >= V) {
            cout << input << endl;
        }
    }

    template<Verbosity V = ALL>
    void loge(string input) {
        writefile(input);

        if (verbosity >= V) {
            cerr << "\033[38;2;255;0;0m" <<input << "\033[0m" << endl;
        }
    }

    template<Verbosity V = ALL>
    void logw(string input) {
        writefile(input);

        if (verbosity >= V) {
            cout << "\033[38;2;255;255;0m" << input << "\033[0m" << endl;
        }
    }
}  // namespace P8
