#pragma once
#include <stdlib.h>
#define MALLOC_CAP_SPIRAM 0
#define heap_caps_malloc(sz, caps) malloc(sz)
