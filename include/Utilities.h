#ifndef UTILITIES_H
#define UTILITIES_H

#include "Parameters.h"

SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order);
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order, int);
STATE_ORDER operator++(STATE_ORDER& order);
STATE_ORDER operator++(STATE_ORDER& order, int);

#endif