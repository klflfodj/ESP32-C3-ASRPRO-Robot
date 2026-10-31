#include "wifi_data.h"

// wifi 数据表
// 注意：仓库里只保留占位符，不要把真实 SSID / 密码提交到公开仓库。
//      表项数量决定轮询的 AP 个数；本机填写真实值后重新烧录即可。
const wifi_data_t wifi_table[] =
{
    //{"SSID", "PASSWORD"},
    {"YOUR_SSID_1", "YOUR_PASSWORD_1"},
    {"YOUR_SSID_2", "YOUR_PASSWORD_2"},
    {"YOUR_SSID_3", "YOUR_PASSWORD_3"},
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
