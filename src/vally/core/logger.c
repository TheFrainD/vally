#include <vally/core/logger.h>

b8 logger_init() {
  FILE *file = fopen("logs/vally.log", "w+");
  if (!file) {
    log_error("Could not open log file!");
    return FALSE;
  }
  log_add_fp(file, 0);
  return TRUE;
}