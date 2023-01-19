#include <iostream>
#include "AbstractSubsystem.h"

int main(int argc, char** argv){
    std::cout << "maiiiinnnn...\n";
    std::string name = "CdTe";
    AbstractSubsystem sys = AbstractSubsystem(name);

    std::cout << "AbstractSubsystem name:\t " << sys.name << std::endl;

    return 0;
}