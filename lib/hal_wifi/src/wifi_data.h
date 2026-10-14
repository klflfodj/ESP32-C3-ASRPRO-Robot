#pragma once

#include <stdint.h>
#include <string.h>

typedef struct
{
    char ssid[64];
    char password[64];
} wifi_data_t;

extern const wifi_data_t wifi_table[];
extern const uint8_t wifi_table_size;

const char* ssid_to_password(const char* ssid);
const char* password_to_ssid(const char* password);



