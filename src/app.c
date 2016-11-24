#include "app.h"
#include "ucmd.h"


void app_handler1(uint8* data, uint8 len)
{
    ucmd_print("Command: name=%s, param=", "APP1");
    if (len == 0) {
        ucmd_print("NULL");   
    }
    else {
        uint8 i;
        for (i = 0; i < len; i++) {
            ucmd_print("%c", data[i]);   
        }
    }
    ucmd_print("\r\n");
}

void app_handler2(uint8* data, uint8 len)
{
    ucmd_print("Command: name=%s, param=", "APP2");
    if (len == 0) {
        ucmd_print("NULL");   
    }
    else {
        uint8 i;
        for (i = 0; i < len; i++) {
            ucmd_print("%c", data[i]);   
        }
    }
    ucmd_print("\r\n");
}

void app_handler3(uint8* data, uint8 len)
{
    ucmd_print("Command: name=%s, param=", "APP3");
    if (len == 0) {
        ucmd_print("NULL");   
    }
    else {
        uint8 i;
        for (i = 0; i < len; i++) {
            ucmd_print("%c", data[i]);   
        }
    }
    ucmd_print("\r\n");
}

void app_init(void)
{
    ucmd_registerHandler("APP1", app_handler1);
    ucmd_registerHandler("APP2", app_handler2);
    ucmd_registerHandler("APP3", app_handler3);
}





