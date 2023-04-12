#ifndef _RANDOM_H_

#define _RANDOM_H_

#include "param.h"

struct rand_state {
    int a;
};

int getrand(struct rand_state *state);

#endif // _RANDOM_H_