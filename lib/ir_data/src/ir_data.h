#pragma once

#include <stdint.h>
#include <string.h>

typedef struct
{
    char temp[8];
    uint32_t code;

} temp_code_t;


extern const temp_code_t tempTable[];
extern const int tempTableSize;

uint32_t tempToCode(const char* temp);
const char* codeToTemp(uint32_t code);



