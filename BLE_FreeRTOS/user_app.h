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
void registerUserAppTasks(void);
int initSpi(void);
int initUART(void);
void spiSendLedStripFrame(void);
void SPI_Callback(mxc_spi_req_t *req, int error);
void SPI_IRQHandler(void);
fptr_U8_t spi_send(uint8_t data);
#endif