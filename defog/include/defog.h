#ifndef DEFOG_H
#define DEFOG_H

#include "esp_log.h"
#include "esp_adc.h"
#include <stdbool.h>
#include "mock_esp_idf_common.h"

#define DUTY_OFF 0
#define DUTY_100_PERCENT 8191

#define TEMPERATURE_ERROR -127.0f

#define NTC_CHANNEL    ADC_CHANNEL_4 // (GPIO32)
#define NTC_B_DEFAULT 3950.0f
#define NTC_R0_DEFAULT 10000.0f
#define NTC_T0_DEFAULT 298
#define NTC_T0_DEFAULT2 298.15f
#define NTC_VCC_DEFAULT 3300.0f

#define DEFOG_SP_DEFAULT    28.0f
#define DEFOG_H_DEFAULT     5.0f
#define DEFOG_DUTY_DEFAULT  20

#ifdef TEST
float defog_get_setpoint(void);
float defog_get_hist(void);
int defog_get_duty_percent(void);
bool defog_get_heater_active(void);
#endif

void defog_setup(float setPoint, float hist, int dutyPercent, int heater_io);
esp_err_t defog_init_heater(float t_inicial);
void defog_set_config(float setPoint, float hist, int dutyPercent);
float defog_get_temperature();
esp_err_t defog_set_state(float temperature);
bool defog_get_state();
int defog_get_current_duty();

#endif