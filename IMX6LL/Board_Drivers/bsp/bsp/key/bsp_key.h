
#pragma once

#include "imx6ul.h"

enum key_value
{
    KEY_NONE = 0,
    KEY0_VALUE,
};

void key_init(void);
int32_t key_get_value(void);

