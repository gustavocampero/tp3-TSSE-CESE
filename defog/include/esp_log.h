#ifndef ESP_LOG_H
#define ESP_LOG_H

#include <stdio.h>

// Macros de logueo simuladas
#define ESP_LOGE(TAG, fmt, ...) printf("[E][%s] " fmt "\n", TAG, ##__VA_ARGS__)
#define ESP_LOGW(TAG, fmt, ...) printf("[W][%s] " fmt "\n", TAG, ##__VA_ARGS__)
#define ESP_LOGI(TAG, fmt, ...) printf("[I][%s] " fmt "\n", TAG, ##__VA_ARGS__)
#define ESP_LOGD(TAG, fmt, ...) printf("[D][%s] " fmt "\n", TAG, ##__VA_ARGS__)
#define ESP_LOGP(TAG, fmt, ...) printf("[P][%s] " fmt "\n", TAG, ##__VA_ARGS__)
#define ESP_LOGN(TAG, fmt, ...) printf("[N][%s] " fmt "\n", TAG, ##__VA_ARGS__)

// Macro dummy para compatibilidad
#define esp_err_to_name(x) "ESP_ERR"

#endif // MOCK_ESP_LOG_H