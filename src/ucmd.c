#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ucmd.h"
#include "CyUart.h"
#include "CyUart_SPI_UART.h"
#include "CyUartClk.h"


cmd_handler handlerTable[MAX_COMMAND_SIZE];
uint8       handlerCount = 0;

task_listener taskListener;


void ucmd_registerHandler(char* name, void (*handler)(uint8*, uint8))
{
//    ucmd_print("CMD Name: %s\r\n", name);
    if (handlerCount < MAX_COMMAND_SIZE) {
        handlerTable[handlerCount].handler = handler;
        memset(handlerTable[handlerCount].name.ch, 0, 5);
        memcpy(handlerTable[handlerCount].name.ch, name, strlen(name));
        handlerTable[handlerCount].name.len = strlen(name);
        
        handlerCount++;
    }
}

void ucmd_parse(command* cmd, uint8* data, uint8 len)
{
    char name[STRING_CAPACITY] = {0};
    
    if (len < 2) {
        return;   
    }
    
    uint8 actLen = len;
    if (data[len-1] == '\r') {
        actLen = len - 1;
    }
    
    uint8 i;
    for (i = 0; i < actLen; i++) {
        if (data[i] != COMMAND_FORMAT_EQUAL) {
            name[i] = data[i];   
            if (i >= STRING_CAPACITY) {
                err("Invalid name");
                return;
            }
        }
        else {
            break;   
        }
    }
    
    int16 idx = -1;
    uint8 j;
    for (j = 0; j < MAX_COMMAND_SIZE; j++) {
        if (i == handlerTable[j].name.len) {
            if (!strncmp(name, handlerTable[j].name.ch, handlerTable[j].name.len)) {
                idx = j;
                break;
            }
        }        
    }
    if (idx >= 0) {
        cmd->handler = handlerTable[idx].handler; 
    } 
    else {
        err("Can't find the command.\r\n");   
        return;
    }
    
    uint8 following = actLen - i;
    if (following <= 1) {
        cmd->len = 0;
    }
    else {
        cmd->len = following - 1;
        memcpy(cmd->data, &data[i+1], cmd->len);
    }    
}

void ucmd_pushTask(task_listener* listener, command* cmd)
{
    if (listener->count < MAX_QUEUE_DEPTH) {
        listener->cmd[(listener->count + listener->head) % MAX_QUEUE_DEPTH] = *cmd;
        listener->count++;
    }
    else {
        err("Queue is overflow");
    }
}

void ucmd_loopTask(task_listener* listener)
{
    if (listener->count > 0) {
        command* cmd = &listener->cmd[listener->head];
        cmd->handler(cmd->data, cmd->len);
        
        if (listener->count == 1) {
            listener->count = 0;
            listener->head = 0;
        }
        else {
            listener->head++;
            listener->count--;            
        }
    }
}

CY_ISR(uartInterruptHandler)
{
	/* RX Buffer Not Empty interrupt */
    uint32 rxInterruptSource = CyUart_GetRxInterruptSourceMasked();
	if (rxInterruptSource & CyUart_INTR_RX_NOT_EMPTY) {
	}
    else if (rxInterruptSource & CyUart_INTR_RX_FULL) {
    }
    
    /* !!We don't clear Rx interrupt here since it's done in generated source.!! */

	/* TX UART Done interrupt.*/
	if (CyUart_GetTxInterruptSourceMasked() & CyUart_INTR_TX_UART_DONE) {
		CyUart_ClearTxInterruptSource(CyUart_INTR_TX_UART_DONE);
    }
}

void ucmd_run(void)
{
    static uint8 data[64] = {0};
    static uint8 len = 0;
    static _Bool isCmd = 0;
    
    if (CyUart_SpiUartGetRxBufferSize() != 0) {
        uint8 byte = CyUart_UartGetByte();
        
        if (len == 0 && !isCmd) {
            if (byte == COMMAND_FORMAT_HEAD) {
                isCmd = 1;
            }
            else {
                data[len++] = byte;
            }
        }
        else {
            if (byte == COMMAND_FORMAT_TAIL) {
                if (isCmd) {
                    command cmd = {NULL, {0}, 0};
                    ucmd_parse(&cmd, data, len);
                    if (cmd.handler != NULL) {
                        ucmd_pushTask(&taskListener, &cmd);
                    }
                    isCmd = 0;
                    len = 0;
                }
                else {
                    ucmd_print("Message: ");
                    uint8 i;
                    for (i = 0; i < len; i++) {
                        ucmd_print("%c", data[i]);   
                    }
                    ucmd_print("\r\n");
                    
                    len = 0;
                }
            }
            else {
                data[len++] = byte;               
            }
        }        
    }

    ucmd_loopTask(&taskListener);
}

void ucmd_start(uint32 baudRate)
{
    uint8  uartOverSampling = CyUart_UART_OVS_FACTOR;        /* CyUart_SPI_UART.h */
    uint32 hfClkFreq        = CYDEV_BCLK__SYSCLK__HZ;        /* cyfitter.h */
	
    /* HFCLCK should be multiples of overSampling value */
    CYASSERT(hfClkFreq % uartOverSampling == 0);
    
    /* Divider = IntegerDivider + FractionalDivider  */   
	uint32 integralDivider = (hfClkFreq / uartOverSampling) / baudRate;

	/* Fractional divider: it's multiples of 32 */
	uint32 fractionalDivider;
	uint32 n1 = (hfClkFreq / uartOverSampling) % baudRate;
	if (n1 == 0) {
		fractionalDivider = 0;
	} else {
        /* Get clock fractional divider. Refer to datasheet of clk component */
		uint32 n2 = baudRate / n1;
		fractionalDivider = 32 / n2;
	}

    CyUartClk_Start();          /* Start UART clock */
	/* Set UART clock frequency. */
	CyUartClk_SetFractionalDividerRegister((uint16) (integralDivider - 1), fractionalDivider);    
    
    CyUart_SetCustomInterruptHandler(uartInterruptHandler);    
    CyUart_Start();
    
    CyUart_UartPutString("START\r\n");
}

void ucmd_print(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);          /* printf perform at '\n' */
    va_end(args);
    
    /* Print all data in buffer, even not ended with '\n'.
     * Note, this funciton is only valid in GCC compiler.
     */    
    fflush(stdout);
}

int _write(int file, char *ptr, int len)
{
    (void) file;
    int i;
    for (i = 0; i < len; i++) {
        CyUart_UartPutChar(*ptr++);
    }
    
    while(0 != (CyUart_SpiUartGetTxBufferSize() + CyUart_GET_TX_FIFO_SR_VALID));          /* Make sure all data in buffer is sent */
    return len;
}


