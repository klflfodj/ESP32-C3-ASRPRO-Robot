#include "ir_data.h"  

const temp_code_t tempTable[] = 
{
    {"17",0xB23F00},
    {"18",0xB23F10},
    {"19",0xB23F30},
    {"20",0xB23F20},
    {"21",0xB23F60},
    {"22",0xB23F70},
    {"23",0xB23F50},
    {"24",0xB23F40},
    {"25",0xB23FC0},
    {"26",0xB23FD0},
    {"27",0xB23F90},
    {"OFF",0xB27BE0},
};

const int tempTableSize = sizeof(tempTable) / sizeof(tempTable[0]);

uint32_t tempToCode(const char* temp) 
{
    if (temp == NULL || temp[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < tempTableSize; i++) 
    {
        if (strcmp(tempTable[i].temp,temp) == 0) 
        {
            return tempTable[i].code;
        }
    }
    return 0;
}

const char* codeToTemp(uint32_t code) 
{
    if (code == 0) 
    {
        return NULL;
    }

    for (int i = 0; i < tempTableSize; i++) 
    {
        if (tempTable[i].code == code) 
        {
            return tempTable[i].temp;
        }
    }
    return NULL;
}







