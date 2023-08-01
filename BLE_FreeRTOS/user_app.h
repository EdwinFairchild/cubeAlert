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

void registerUserAppTasks(void);
void initSpi(void);
void initUART(void);

#endif