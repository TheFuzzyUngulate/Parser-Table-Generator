#ifndef RESULT_H
#define RESULT_H
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



enum p_state {

};

enum p_tokens {

};

enum p_act_name {
    GOTO = 0,
    SHIFT = 1,
    REDUCE = 2,
    ERROR = 3,
    ACCEPT = 4
};

struct p_action {
    p_state next;
    p_act_name name;
    void (*action)();

};

#endif