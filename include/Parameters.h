#pragma once
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>

// Debugging
static bool DEBUG = true;

// Timing
// DON'T CHANGE THIS WITHOUT EXTENSIVE TESTING
static const unsigned short         PERIOD          = 1;

// IP addresses
static std::string                  LOCAL_IP        = "192.168.1.8";
static const unsigned short         LOCAL_PORT      = 9999;

static std::string                  GSE_IP          = "192.168.1.100";
static const unsigned short         GSE_PORT        = 9999;

static const std::string            TLM_IP          = "127.0.0.1";
static const unsigned short         TLM_PORT        = 0;

static const std::string            SPMU_IP         = "127.0.0.1";
static const unsigned short         SPMU_PORT       = 0;

static const std::string            PLENUM_IP       = "127.0.0.1";
static const unsigned short         PLENUM_PORT     = 0;

// loop order for subsystems:
enum SUBSYSTEM_ORDER {
    HOUSEKEEPING,
    CDTE_1,
    CMOS_1,
    CDTE_2,
    TIMEPIX,
    CDTE_3,
    CMOS_2,
    CDTE_4,
    SUBSYSTEM_COUNT
};

// loop order for states:
enum STATE_ORDER {
    CMD_SEND,
    DATA_REQ,
    DATA_RECV,
    DATA_CHECK,
    DATA_STORE,
    IDLE,
    STATE_COUNT
};

#endif