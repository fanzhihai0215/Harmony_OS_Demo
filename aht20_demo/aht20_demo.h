#ifndef AHT20_DEMO_H
#define AHT20_DEMO_H

#include <stdint.h>

uint32_t AHT20_Calibrate(void);

uint32_t AHT20_StartMeasure(void);

uint32_t AHT20_GetMeasureResult(float* temp, float* humi);

#endif  // AHT20_DEMO_H