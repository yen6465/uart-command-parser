#ifndef __UCMD_H__
#define __UCMD_H__
    
#include "cytypes.h"

#define STRING_CAPACITY         (5u)    
#define MAX_COMMAND_DATA_SIZE   (64u)
#define MAX_QUEUE_DEPTH         (5u)    
#define MAX_COMMAND_SIZE        (32u)
    
typedef struct {
    char     ch[STRING_CAPACITY];
    uint8    len;
} string;
    
typedef struct {
    string   name;
    void     (*handler)(uint8* data, uint8 len);
} cmd_handler;
    
typedef struct {
    void     (*handler)(uint8* data, uint8 len);
    uint8    data[MAX_COMMAND_DATA_SIZE];
    uint8    len;
} command;

enum {
    COMMAND_FORMAT_HEAD      = (0x40),  /* '@'  */    
    COMMAND_FORMAT_TAIL      = (0x0A),  /* '\n' */  
    COMMAND_FORMAT_DELIMITER = (0x2C),  /* ','  */  
    COMMAND_FORMAT_EQUAL     = (0x3D),  /* '='  */      
};

typedef struct {
	command  cmd[MAX_QUEUE_DEPTH];
	uint8    head;
	uint8    count;
} task_listener;
    
void ucmd_run(void);
void ucmd_start(uint32 baudRate);
void ucmd_print(const char* format, ...);
void ucmd_registerHandler(char* name, void (*handler)(uint8*, uint8));
#define p(...)      ucmd_print(__VA_ARGS__)
#define err(...)    do{ucmd_print("Error: ");ucmd_print(__VA_ARGS__);}while(0)
    
#endif
    