#include "defog.h"
#include <math.h>
#include "ledc.h"

static const char* TAG = "DEFOG";
static const int verb = 0;

static bool heater_active = false;

adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t cali_handle = NULL;

float defog_SetPoint = DEFOG_SP_DEFAULT;
float defog_Hist = DEFOG_H_DEFAULT;
int defog_Duty  = DEFOG_DUTY_DEFAULT;

#ifdef TEST
float defog_get_setpoint(void) { return defog_SetPoint; }
float defog_get_hist(void) { return defog_Hist; }
int defog_get_duty_percent(void) { return defog_Duty; }
bool defog_get_heater_active(void) { return heater_active; }
#endif

static int calculate_duty(float t) {
    int ret;
    float threshold1 = defog_SetPoint - defog_Hist;
    float duty_75 = (float)(defog_Duty) * 0.75f;
    float duty_50 = (float)(defog_Duty) * 0.5f;

    if (t < 0.0f) {
        ret = defog_Duty;
    } else if (t >= 0.0f && t < threshold1) {
        float slope = (duty_75 - (float)(defog_Duty)) / (threshold1 - 0.0f);  // pendiente negativa
        ret = (int)((float)(defog_Duty) + slope * (t - 0.0f));
    } else if (t >= threshold1 && t <= defog_SetPoint) {
        float slope = (duty_50 - duty_75) / (defog_SetPoint - threshold1);  // también negativa
        ret = (int)(duty_75 + slope * (t - threshold1));
    } else {
        ret = DUTY_OFF;
    }

    return (int)(ret*DUTY_100_PERCENT/100);
}

static float read_temperature_NTC()
{
    int adc_raw = 0;
    int voltage_mV = 0;
    float voltageSum = 0;
    float T = TEMPERATURE_ERROR;

    for (size_t i = 0; i < 5; i++)
    {
        esp_err_t ret = adc_oneshot_read(adc_handle, NTC_CHANNEL, &adc_raw);
        if (ret != ESP_OK) {
            printf("Error reading ADC: %s", esp_err_to_name(ret));
            T = TEMPERATURE_ERROR;
            return T;
        }

        adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage_mV);
        float voltage = voltage_mV;

        if (voltage >= 2870.0)
        {
            printf("Voltage out of range: %.2f mV", voltage);
            T = TEMPERATURE_ERROR;
            return T;
        }

        voltageSum += voltage;
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    float voltageAvg = voltageSum / 5.0;
    float r_ntc = 10000.0 * (voltageAvg / (NTC_VCC_DEFAULT - voltageAvg));
    float temp_k = (NTC_B_DEFAULT / (log(r_ntc / NTC_R0_DEFAULT) + (NTC_B_DEFAULT / NTC_T0_DEFAULT2)));
    // float temp_k = (NTC_B_DEFAULT / ((r_ntc / NTC_R0_DEFAULT) + (NTC_B_DEFAULT / NTC_T0_DEFAULT2)));
    float temp_c = temp_k - 273.15;

    // Validar resultado de la temperatura
    if (isnan(temp_c) || temp_c < -40.0)
    {
        printf("Error en cálculo de temperatura: %.2f °C", temp_c);
        T = TEMPERATURE_ERROR;
        return T;
    }

    // Actualizar temperatura
    T = temp_c;

    printf("Voltaje: %.2f mV | Temperatura: %.1f °C", voltageAvg, T);
    return T;
}

static void set_heater_duty(int duty)
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, (uint32_t)(duty));
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
}

float defog_get_temperature()
{
    return read_temperature_NTC();
}

void defog_setup(float setPoint, float hist, int dutyPercent, int heater_io)
{
    defog_SetPoint = setPoint;
    defog_Hist = hist;
    defog_Duty = dutyPercent;
    printf("Setting... SP = %.1f  H = %.1f  D = %d",defog_SetPoint, defog_Hist, defog_Duty);

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc_handle, NTC_CHANNEL, &chan_config);

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);

    /* ============================================= */

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
         .timer_num = LEDC_TIMER_0,            // timer index
        .freq_hz = 200,                      // frequency of PWM signal
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = heater_io,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&ledc_channel);
}

void defog_set_config(float setPoint, float hist, int dutyPercent)
{
    defog_SetPoint = setPoint;
    defog_Hist = hist;
    defog_Duty = dutyPercent;
    printf("Setting... SP = %.1f  H = %.1f  D = %d",defog_SetPoint, defog_Hist, defog_Duty);
}

esp_err_t defog_init_heater(float t_inicial)
{
    if(t_inicial == TEMPERATURE_ERROR) {
        return ESP_FAIL;
    }

    if (t_inicial < (defog_SetPoint)) {
        int duty = calculate_duty(t_inicial);
        printf("Defog handler started ::: Defog ACTIVADO! (%d %%)",duty*100/DUTY_100_PERCENT);
        set_heater_duty(duty);
        heater_active = true;
    } else {
        printf("Defog handler started ::: Defog DESACTIVADO!");
        set_heater_duty(DUTY_OFF);
        heater_active = false;
    }

    return ESP_OK;
}

esp_err_t defog_set_state(float temperature)
{
    if(temperature == TEMPERATURE_ERROR) {
        return ESP_FAIL;
    }

    if (temperature < (defog_SetPoint - defog_Hist))
    {
        int duty = calculate_duty(temperature);
        printf("Defog ACTIVADO! (%d %%)",duty*100/DUTY_100_PERCENT);
        set_heater_duty(duty);
        heater_active = true;
    }
    else if (temperature < defog_SetPoint && temperature >= (defog_SetPoint - defog_Hist) && heater_active)
    {
        int duty = calculate_duty(temperature);
        printf("Defog (%d %%)",duty*100/DUTY_100_PERCENT);
        set_heater_duty(duty);
        heater_active = true;
    }
    else if (temperature >= defog_SetPoint && heater_active)
    {
        // Apagar el circuito si la temperatura supera o alcanza el SET_POINT
        printf("Defog DESACTIVADO!");
        set_heater_duty(DUTY_OFF);
        heater_active = false;
    }
}

bool defog_get_state()
{
    uint32_t duty = ledc_get_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    return (duty > 0);
}

int defog_get_current_duty()
{
    uint32_t duty = ledc_get_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    return duty;
}

void defog_fix_duty(int duty_percent)
{
    if (duty_percent < 0.0f) duty_percent = 0.0f;
    if (duty_percent > 100.0f) duty_percent = 100.0f;
    uint32_t duty = (uint32_t)((duty_percent / 100.0f) * DUTY_100_PERCENT);
    set_heater_duty(duty);
}