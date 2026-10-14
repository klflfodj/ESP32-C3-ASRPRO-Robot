#include "wifi_data.h"

// wifi 数据表
const wifi_data_t wifi_table[] =
{
    //{"SSID", "PASSWORD"},
    {"120", "1234567890"},
    {"118", "1234567890"},
    {"ry400", "000000001"},
};

// wifi 数据表大小
const uint8_t wifi_table_size = sizeof(wifi_table) / sizeof(wifi_table[0]); 

// 通过 wifi 名获取 wifi 密码
const char* ssid_to_password(const char* ssid)
{
    if (ssid == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < wifi_table_size; i++)
    {
        
        if (strcmp(wifi_table[i].ssid,ssid) == 0)
        {
            return wifi_table[i].password;
        }
    }
    return NULL;
}

//通过 wifi 密码获取 wifi 名
const char* password_to_ssid(const char* password)
{
    for (int i = 0; i < wifi_table_size; i++)
    {
        if (strcmp(wifi_table[i].password,password) == 0)
        {
            return wifi_table[i].ssid;
        }
    }
    return NULL;
}
