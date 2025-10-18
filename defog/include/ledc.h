#ifndef LEDC_H
#define LEDC_H

#include "stdint.h"
#include "esp_err.h"

// ==========================
//  Constantes simuladas
// ==========================
#define LEDC_HIGH_SPEED_MODE 0
#define LEDC_CHANNEL_1       1
#define LEDC_TIMER_0         0
#define LEDC_INTR_DISABLE    0
#define LEDC_TIMER_13_BIT    13
#define LEDC_AUTO_CLK        0

// ==========================
//  Tipos simulados
// ==========================
typedef int ledc_mode_t;
typedef int ledc_channel_t;
typedef int ledc_timer_t;
typedef int ledc_intr_type_t;
typedef int ledc_clk_cfg_t;

typedef struct {
    ledc_mode_t speed_mode;
    uint32_t duty_resolution;
    ledc_timer_t timer_num;
    uint32_t freq_hz;
    ledc_clk_cfg_t clk_cfg;
} ledc_timer_config_t;

typedef struct {
    int gpio_num;
    ledc_mode_t speed_mode;
    ledc_channel_t channel;
    ledc_intr_type_t intr_type;
    ledc_timer_t timer_sel;
    uint32_t duty;
    uint32_t hpoint;
} ledc_channel_config_t;

// ==========================
//  Funciones simuladas
// ==========================
esp_err_t ledc_timer_config(const ledc_timer_config_t *config);
esp_err_t ledc_channel_config(const ledc_channel_config_t *config);
esp_err_t ledc_set_duty(ledc_mode_t speed_mode, ledc_channel_t channel, uint32_t duty);
esp_err_t ledc_update_duty(ledc_mode_t speed_mode, ledc_channel_t channel);
uint32_t ledc_get_duty(ledc_mode_t speed_mode, ledc_channel_t channel);

#endif // MOCK_DRIVER_LEDC_H