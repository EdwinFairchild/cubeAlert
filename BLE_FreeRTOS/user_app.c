#include "user_app.h"


TaskHandle_t rangeSensorTask_id;
TaskHandle_t ledStripTask_id;

/************| UART STUFF |***************/
#define UART_BAUD 9600
#define UART_BUFF_SIZE 1


/************| LED STUFF |***************/
#define BRIGHTNESS_PREAMBLE (0x07)
#define END_OF_FRAME_PREAMBLE ((uint8_t)0x00)
#define END_OF_FRAME_BYTES 9 // ((NUM_OF_LEDS  - 1) / 16 ) rounded up == 9 
#define START_OF_FRAME ((uint32_t)0x00000000)
#define END_OF_FRAME ((uint32_t)0xFFFFFFFF)
#define NUM_OF_LEDS 144
#define SET_FRAME(a,b,g,r) ( (uint32_t)  ( (BRIGHTNESS_PREAMBLE) << 29 | \
                                                    ((uint8_t)a) << 24 | \
                                                    ((uint8_t)b) << 16 | \
                                                    ((uint8_t)g) << 8 | \
                                                    ((uint8_t)r) ) )
uint16_t measurement_mm = 0;
/***** Globals *****/
volatile int READ_FLAG;

void rangeSensorTask(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(1000);
        APP_TRACE_INFO0("Range Sensor Task");
    }
}

void ledStripTask(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(1000);
        APP_TRACE_INFO0("LED Strip Task");
    }
}
void registerUserAppTasks(void)
{
    APP_TRACE_INFO0(">>> Registering user application tasks...");
    xTaskCreate(rangeSensorTask, "Range Sensor Task", 256, NULL, 1, &rangeSensorTask_id);
    xTaskCreate(ledStripTask, "LED Strip Task", 256, NULL, 1, &ledStripTask_id);
}

void initSpi(void)
{
    APP_TRACE_INFO0(">>> Initializing SPI...");   
}

void initUART(void)
{
    APP_TRACE_INFO0(">>> Initializing UART...");   
}