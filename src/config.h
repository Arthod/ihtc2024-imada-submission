#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG 1 // 0: No debug, 1: Console verbose, 2: Delta debug, 3: Extreme delta debug.
#define DEBUG_VERBOSE 1 // 0: No verbose, 1: Allow verbose. (Actual verbose is controlled by debug_level parameter for main.cpp)

// Algorithms running times in seconds. These values are not exact, please see main.cpp for more details.
#define RUNTIME_ALG1 530 // Running time of SA in seconds.
#define RUNTIME_ALG2 65 // Running time of ILS in seconds.

#endif