#include "Utilities.h"

SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order) {
    order = static_cast<SUBSYSTEM_ORDER>((order + 1) % SUBSYSTEM_COUNT);
    return order;
}

SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order, int) {
    SUBSYSTEM_ORDER result = order;
    ++order;
    return result;  
}

STATE_ORDER operator++(STATE_ORDER& order) {
    order = static_cast<STATE_ORDER>((order + 1) % STATE_COUNT);
    return order;
}

STATE_ORDER operator++(STATE_ORDER& order, int) {
    STATE_ORDER result = order;
    ++order;
    return result;  
}

int inc_mod(int i, int n) {
    return i = (i + 1 == n ? 0: i + 1);
}