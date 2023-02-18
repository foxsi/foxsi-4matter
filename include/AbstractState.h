#ifndef ABSTRACTSTATE_H
#define ABSTRACTSTATE_H

#include "Parameters.h"
#include <string>

class AbstractState {
    public:
        STATE_ORDER state;
        
        AbstractState();
        virtual ~AbstractState() = 0;

        virtual void enter() = 0;
        virtual void exit() = 0;
        virtual void update() = 0;
};

#endif