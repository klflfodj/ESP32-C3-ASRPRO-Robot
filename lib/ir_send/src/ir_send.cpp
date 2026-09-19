#include "ir_send.h"

#include <IRremote.h>

bool ir_init(ir_send_t* send, const ir_send_config_t* cfg)
{
    if (send == NULL || cfg == NULL)
    {
        return false;
    }

    send->cfg = *cfg;

    IrSender.begin(send->cfg.ir_pin);
    
    return true;
}

void ir_send_temp(const uint32_t temp)
{
    if (temp == 0) 
    {
        return;
    }
    IrSender.sendNECMSB(temp, 24);
}



