#include <iostream>
#include <vector>
#include <string>

#include <nlohmann/json.hpp>

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
        std::vector<std::tuple<std::string, Functor>> tests;
        virtual void init(){}
    public:
        void add_test(std::string name, Functor test){
            std::tuple<std::string, Functor> t{name, test};
            tests.push_back(t);
        }

        void run(){
            for(auto [name, functor] : tests){
                std::cout << "......\tRunning test [" << name << "]" << std::endl;
                init();
                functor();
            }
        }
};