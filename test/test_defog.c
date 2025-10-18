#include "unity.h"
#include "mock_ledc.h"
#include "mock_esp_adc.h"
#include "mock_esp_log.h"
#include "mock_esp_err.h"
#include "mock_esp_idf_common.h"
#include "defog.h"

void setUp(void) {}
void tearDown(void) {}

void test_config_por_defecto_cuando_inicia(void)
{
    TEST_ASSERT_EQUAL_FLOAT(DEFOG_SP_DEFAULT, defog_get_setpoint());
    TEST_ASSERT_EQUAL_FLOAT(DEFOG_H_DEFAULT, defog_get_hist());
    TEST_ASSERT_EQUAL_INT(DEFOG_DUTY_DEFAULT, defog_get_duty_percent());
}

void test_defog_set_config_asigna_los_valores_correctos(void)
{
    float nuevoSP = 30.5f;
    float nuevoHist = 3.5f;
    int nuevoDuty = 40;

    defog_set_config(nuevoSP, nuevoHist, nuevoDuty);

    TEST_ASSERT_EQUAL_FLOAT(nuevoSP, defog_get_setpoint());
    TEST_ASSERT_EQUAL_FLOAT(nuevoHist, defog_get_hist());
    TEST_ASSERT_EQUAL_INT(nuevoDuty, defog_get_duty_percent());
}

void test_init_heater_falla_si_temperatura_invalida(void)
{
    esp_err_t ret = defog_init_heater(TEMPERATURE_ERROR);

    TEST_ASSERT_EQUAL(ESP_FAIL, ret);
}

void test_heater_activa_si_temperatura_menor_a_setpoint_cuando_inicio(void)
{
    defog_set_config(25.0f, 5.0f, 20);

    ledc_set_duty_IgnoreAndReturn(ESP_OK);
    ledc_update_duty_IgnoreAndReturn(ESP_OK);
    esp_err_t ret = defog_init_heater(24.9f);

    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_TRUE(defog_get_heater_active());
}

void test_heater_no_activa_si_temperatura_setpoint_cuando_inicio(void)
{
    defog_set_config(25.0f, 5.0f, 20);

    ledc_set_duty_IgnoreAndReturn(ESP_OK);
    ledc_update_duty_IgnoreAndReturn(ESP_OK);
    esp_err_t ret = defog_init_heater(25.0f);

    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_FALSE(defog_get_heater_active());
}

void test_defog_set_state_devuelve_fail_con_temperatura_invalida(void)
{
    esp_err_t ret = defog_set_state(TEMPERATURE_ERROR);
    TEST_ASSERT_EQUAL(ESP_FAIL, ret);
}

void test_defog_set_state_devuelve_ok_con_temperatura_valida(void)
{
    ledc_set_duty_IgnoreAndReturn(ESP_OK);
    ledc_update_duty_IgnoreAndReturn(ESP_OK);

    esp_err_t ret = defog_set_state(0.0f);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

void test_heater_se_activa_si_temperatura_baja(void)
{
    defog_set_config(25.0f, 5.0f, 20);

    ledc_set_duty_IgnoreAndReturn(ESP_OK);
    ledc_update_duty_IgnoreAndReturn(ESP_OK);

    defog_set_state(19.9f);

    TEST_ASSERT_TRUE(defog_get_heater_active());

    // Simulamos que el duty interno es > 0 (heater activo)
    ledc_get_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 1200);

    TEST_ASSERT_TRUE(defog_get_state());
}

void test_heater_duty_maximo_si_se_activa_si_temperatura_menor_a_cero(void)
{
    defog_set_config(25.0f, 5.0f, 20);

    ledc_set_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0.2 * DUTY_100_PERCENT, ESP_OK);
    ledc_update_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, ESP_OK);
    defog_set_state(-0.1f); // dentro del rango, sigue encendido

    TEST_ASSERT_TRUE(defog_get_heater_active());

    // Simulamos que el duty interno es > 0 (heater activo)
    ledc_get_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 1200);

    TEST_ASSERT_TRUE(defog_get_state());
}

void test_heater_duty_calculado_si_se_activa_si_temperatura_baja_mayor_a_cero(void)
{
    float setpoint = 25.0f;
    float hist = 5.0f;
    int duty_base = 20;
    float t = 19.9f;

    float threshold1 = setpoint - hist;
    float duty_75 = duty_base * 0.75f;
    float slope = (duty_75 - duty_base) / (threshold1 - 0.0f);
    int ret = (int)(duty_base + slope * t);
    int esperado = (int)(ret * DUTY_100_PERCENT / 100.0f);

    defog_set_config(setpoint, hist, duty_base);

    ledc_set_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, esperado, ESP_OK);
    ledc_update_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, ESP_OK);
    defog_set_state(t); // dentro del rango, sigue encendido

    TEST_ASSERT_TRUE(defog_get_heater_active());

    // Simulamos que el duty interno es > 0 (heater activo)
    ledc_get_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 1200);

    TEST_ASSERT_TRUE(defog_get_state());
}

void test_heater_queda_activo_si_temperatura_dentro_del_rango_h_sp_luego_de_temperatura_baja (void)
{
    defog_set_config(25.0f, 5.0f, 20);

    ledc_set_duty_IgnoreAndReturn(ESP_OK);
    ledc_update_duty_IgnoreAndReturn(ESP_OK);
    defog_set_state(19.9f); // activa

    defog_set_state(23.0f); // dentro del rango, sigue encendido

    TEST_ASSERT_TRUE(defog_get_heater_active());

    ledc_get_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 1000);
    TEST_ASSERT_TRUE(defog_get_state());
}

void test_heater_activa_con_duty_calculado_si_temperatura_dentro_del_rango_h_sp_luego_de_temperatura_baja (void)
{
    float setpoint = 25.0f;
    float hist = 5.0f;
    int duty_base = 20;
    float t = 23.0f;

    float threshold1 = setpoint - hist;
    float slope = duty_base * (0.5f - 0.75f) / (setpoint - threshold1);
    float ret_percent = duty_base * 0.75f + slope * (t - threshold1);
    int esperado = (int)(ret_percent * DUTY_100_PERCENT / 100.0f);

    defog_set_config(setpoint, hist, duty_base);

    ledc_set_duty_ExpectAnyArgsAndReturn(ESP_OK);
    ledc_update_duty_ExpectAnyArgsAndReturn(ESP_OK);
    defog_set_state(19.9f); // activa

    ledc_set_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, esperado, ESP_OK);
    ledc_update_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, ESP_OK);
    defog_set_state(t); // dentro del rango, sigue encendido

    TEST_ASSERT_TRUE(defog_get_heater_active());

    ledc_get_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 1000);
    TEST_ASSERT_TRUE(defog_get_state());
}

void test_heater_se_apaga_si_temperatura_sp(void)
{
    defog_set_config(25.0f, 5.0f, 20);

    // Luego sube la temperatura a 26 → debe apagarse
    ledc_set_duty_IgnoreAndReturn(ESP_OK);
    ledc_update_duty_IgnoreAndReturn(ESP_OK);
    defog_set_state(25.0f);

    TEST_ASSERT_FALSE(defog_get_heater_active());

    // Simulamos que LEDC ahora tiene duty = 0
    ledc_get_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);

    TEST_ASSERT_FALSE(defog_get_state());
}

void test_heater_queda_apagado_si_temperatura_dentro_del_rango_h_sp_luego_de_temperatura_sp (void)
{
    defog_set_config(25.0f, 5.0f, 20);

    ledc_set_duty_IgnoreAndReturn(ESP_OK);
    ledc_update_duty_IgnoreAndReturn(ESP_OK);
    defog_set_state(25.0f); // activa

    defog_set_state(23.0f); // dentro del rango, sigue encendido

    TEST_ASSERT_FALSE(defog_get_heater_active());

    ledc_get_duty_ExpectAndReturn(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);
    TEST_ASSERT_FALSE(defog_get_state());
}