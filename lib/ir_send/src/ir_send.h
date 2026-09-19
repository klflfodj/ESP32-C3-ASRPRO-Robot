#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct 
{
    uint8_t ir_pin;  //红外发送引脚号
} ir_send_config_t;

typedef struct 
{
    ir_send_config_t cfg;   //红外发送配置参数
} ir_send_t;

bool ir_init(ir_send_t* send , const ir_send_config_t* cfg);
void ir_send_temp(const uint32_t temp);








