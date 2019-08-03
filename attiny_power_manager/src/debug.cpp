#include "debug.h"
#include "ILogger.h"

static ILogger* _logger;

void debug_set_logger(ILogger* logger) {
    _logger = logger;
}

ILogger* debug_get_logger(void) {
    return _logger;
}
