#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <iostream>

namespace sc = boost::statechart;
struct Greeting;

struct Machine : sc::state_machine< Machine, Greeting > {};

struct Greeting : sc::simple_state< Greeting, Machine > 
{
    Greeting() {std::cout << "created!\n";}
    ~Greeting() {std::cout << "destroyed :(\n";}
};


int main(){


    std::cout << "starting..." << std::endl;    
    Machine mach;
    mach.initiate();

    std::cout << "ending.\n";
    
    return 0;
}

