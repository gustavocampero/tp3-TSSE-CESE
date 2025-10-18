#ifndef ESP_ADC_H
#define ESP_ADC_H

#include "esp_err.h"

#define ESP_OK 0
#define ESP_FAIL -1

typedef int adc_oneshot_unit_handle_t;
typedef int adc_cali_handle_t;
typedef int adc_channel_t;
typedef int adc_unit_t;
typedef int adc_atten_t;
typedef int adc_bitwidth_t;

typedef struct {
    adc_unit_t unit_id;
} adc_oneshot_unit_init_cfg_t;

typedef struct {
    adc_atten_t atten;
    adc_bitwidth_t bitwidth;
} adc_oneshot_chan_cfg_t;

typedef struct {
    adc_unit_t unit_id;
    adc_atten_t atten;
    adc_bitwidth_t bitwidth;
} adc_cali_line_fitting_config_t;

esp_err_t adc_oneshot_new_unit(const adc_oneshot_unit_init_cfg_t *init_config,
                               adc_oneshot_unit_handle_t *handle);
esp_err_t adc_oneshot_config_channel(adc_oneshot_unit_handle_t handle,
                                     adc_channel_t channel,
                                     const adc_oneshot_chan_cfg_t *config);
esp_err_t adc_oneshot_read(adc_oneshot_unit_handle_t handle,
                           adc_channel_t channel,
                           int *result);
esp_err_t adc_cali_create_scheme_line_fitting(const adc_cali_line_fitting_config_t *config,
                                              adc_cali_handle_t *out_handle);
esp_err_t adc_cali_raw_to_voltage(adc_cali_handle_t handle, int raw, int *voltage);
#endif