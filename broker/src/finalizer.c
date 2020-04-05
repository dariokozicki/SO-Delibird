#include "finalizer.h"


void finalize(){
    log_info(optional_logger, "Shutting down.");
    config_destroy(config);
    log_destroy(optional_logger);
    log_destroy(obligatory_logger);
    exit(SUCCESS);
}
