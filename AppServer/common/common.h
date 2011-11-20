#include <stdio.h>

// the following defines are for debug printf
#define DEBUG_PREFIX "HoneyBadger debug: "
#define MY_DEBUG 1

#if MY_DEBUG
#define printd( s, arg...) printf(DEBUG_PREFIX s "\n", ##arg)
#else
#define printd( s, arg...)
#endif
