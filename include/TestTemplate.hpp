#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <tuple>

#include "Logger.hpp"

using namespace P8;
using namespace std;

/*
This is a template test, it functions as an abstraction of running a test, and provides an easy implementation interface using lambda expressions
### Methods:
2 public methods:
- add_test(string, Functor) 
  takes a string name for the test, and a template functor, this should be something like std::functor<void()>
- run()
  runs all added tests

1 protected method:
- init()
  it is a virtual function that should be overloaded, here the test specifics for the module can be defined such that it doesn't bloat other module tests and thus slowing down the tests.
*/
template<typename Functor>
class Test {
 protected:
    vector<std::tuple<string, Functor>> tests;
    virtual void init() {}
 public:
    void add_test(string name, Functor test) {
        tuple<string, Functor> t{name, test};
        tests.push_back(t);
    }
    void run(){
        for (auto [name, functor] : tests) {
            log("......\tRunning test [{}]", name);
            init();
            functor();
        }
    }
};
