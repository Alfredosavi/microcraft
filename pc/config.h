#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

extern int DEBUG;

#define DEBUG_PRINT(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "[DEBUG] " fmt, ##__VA_ARGS__); } while (0)

#endif
