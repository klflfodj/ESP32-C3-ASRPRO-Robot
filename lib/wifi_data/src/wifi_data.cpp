#include "wifi_data.h"

const wifi_data_t wifi_data[] = 
{
    //{"SSID", "PASSWORD"},
    {"120", "1234567890"},
    {"118", "1234567890"},
};

const char wifi_ssid_size = sizeof(wifi_data) / sizeof(wifi_data[0]);
const char wifi_password_size = sizeof(wifi_data[0].password);


