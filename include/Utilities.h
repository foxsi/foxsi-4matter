#ifndef UTILITIES_H
#define UTILITIES_H

#include "Parameters.h"

// deprecate these later if they are not used
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order);
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order, int);
STATE_ORDER operator++(STATE_ORDER& order);
STATE_ORDER operator++(STATE_ORDER& order, int);

// increment i mod n (loop through enum?)
int inc_mod(int i, int n);

#endif