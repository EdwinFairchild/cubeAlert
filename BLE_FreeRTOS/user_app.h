#ifndef _USER_APP_H_
#define _USER_APP_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "wsf_trace.h"
#include "board.h"
#include "mxc_delay.h"
#include "mxc_device.h"
#include "uart.h"
#include "nvic_table.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stdlib.h"
#include "uart.h"
#include "spi.h"
#include "sk9822.h"
#include "mxc_delay.h"
void registerUserAppTasks(void);
int initSpi(void);
int initUART(void);
void spiSendLedStripFrame(void);
void SPI_Callback(mxc_spi_req_t *req, int error);
void SPI_IRQHandler(void);
void skSetLed(uint8_t ledNum, uint8_t global, uint8_t r, uint8_t g , uint8_t b);
void clearLedArray(void);
void skUpdateLed(void);
void spiSend(uint8_t data);
void fillLEDs(int value);
void disableUart(void);
void enableUart(void);
void UART3_Handler(void);
void skSetUnmappedLed(uint16_t rangeSensorValue);
#endif 