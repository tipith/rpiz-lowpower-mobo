#pragma once

#include "ILogger.h"

// __PRETTY_FUNCTION__

#define DBG_EXT_PRINTF(f_, ...) debug_get_logger()->tagged_printf(__func__, (f_), ##__VA_ARGS__)

#define DBG_PRINTF(f_, ...) debug_get_logger()->printf((f_), ##__VA_ARGS__)

void debug_set_logger(ILogger* logger);
ILogger* debug_get_logger(void);
