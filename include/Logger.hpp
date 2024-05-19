#pragma once

#include <ostream>
#include <fstream>
#include <string>
#include <utility>

#include "Globals.hpp"
#include "Utils.hpp"

#ifndef __FILE_NAME__
    #define __FILE_NAME__ split_string(__FILE__, "/").back()
#endif

#define start_line(name, line) '[' << name << ':' << line << ']' << '\t'
#define loginit P8::filename = __FILE_NAME__; P8::line = __LINE__;

#define log loginit P8::_log
#define logw loginit P8::_logw
#define loge loginit P8::_loge

using namespace std;

namespace P8 {
    auto verbosity = 0;
    string filename;
    int line;

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

    string color(int r, int g, int b) {
        return format("\033[38;2;{};{};{}m", r, g, b);
    }

    template<Verbosity V = ALL, typename... Args>
    constexpr void _log(format_string<Args...> fmt, Args&&... args) {
        auto input = format(fmt , forward<Args>(args)...);
        writefile(input);

        auto intensity = 0xff - (V * (0xff/(INFO*4)));
        if (verbosity >= V) {
            cout << start_line(filename, line) << color(intensity, intensity, intensity) << input << "\033[0m" << endl;
        }
    }
    template<Verbosity V = ALL>
    constexpr void _log() {
        if (verbosity >= V)
            cout << start_line(filename, line) << endl;
    }

    template<Verbosity V = ALL, typename... Args>
    constexpr void _loge(format_string<Args...> fmt, Args&&... args) {
        auto input = format(fmt, forward<Args>(args)...);
        writefile(input);

        auto intensity = 0xff - (V * (0xff/(INFO*4)));
        if (verbosity >= V) {
            cerr << start_line(filename, line) << color(intensity, 0, 0) << input << "\033[0m" << endl;
        }
    }

    template<Verbosity V = ALL, typename... Args>
    constexpr void _logw(format_string<Args...> fmt, Args&&... args) {
        auto input = format(fmt, forward<Args>(args)...);
        writefile(input);

        auto intensity = 0xff - (V * (0xff/(INFO*4)));
        if (verbosity >= V) {
            cout << start_line(filename, line) << color(intensity, intensity, 0) << input << "\033[0m" << endl;
        }
    }
}  // namespace P8
