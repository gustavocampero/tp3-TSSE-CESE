#ifndef ESP_IDF_COMMON_H
#define ESP_IDF_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// ===== Tipos simulados =====
typedef int gpio_num_t;
typedef uint32_t TickType_t;

typedef int adc_channel_t;
typedef int adc_unit_t;
typedef int adc_atten_t;
typedef int adc_bitwidth_t;

// ===== Macros FreeRTOS =====
#define pdMS_TO_TICKS(ms)   (ms)
#define vTaskDelay(ticks)   ((void)(ticks))

// ===== Constantes ADC =====
#define ADC_CHANNEL_4        4
#define ADC_UNIT_1           1
#define ADC_ATTEN_DB_12     12
#define ADC_BITWIDTH_DEFAULT 10

// ===== Constantes generales =====
#define ESP_OK   0
#define ESP_FAIL -1

#endif // MOCK_ESP_IDF_COMMON_H