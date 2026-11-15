#pragma once
typedef void *SemaphoreHandle_t;
#define xSemaphoreCreateMutex() ((SemaphoreHandle_t)1)
#define xSemaphoreTake(m, t) ((void)(m), 1)
#define xSemaphoreGive(m) ((void)(m))
