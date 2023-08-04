#include "user_app.h"
#include "sk9822.h"
/************| FreeRTOS STUFF |***************/
TaskHandle_t rangeSensorTask_id;
TaskHandle_t ledStripTask_id;
SemaphoreHandle_t xSemaphore = NULL;
/************| UART STUFF |***************/
#define RANGE_SENSOR_UART_BAUD 9600
volatile int READ_FLAG;
uint8_t RxData[1]; // we wil only be using 1 space but just to be safe make it bigger
 mxc_uart_req_t read_req;
uint16_t measurement_mm = 0;
volatile uint32_t delaytVar = 0;
/************| SPI STUFF |***************/
// FTHR board applicable settings
#define SPI MXC_SPI1
#define SPI_IRQ SPI1_IRQn
#define MOSI_PIN 21
#define MISO_PIN 22
#define FTHR_Defined 1
#define DATA_LEN 4 // Words
#define DATA_VALUE 0xA5A5 // This is for master mode only...
#define VALUE 0xFFFF
#define SPI_SPEED 10000000 // Bit Rate
volatile mxc_spi_req_t req;
mxc_spi_pins_t spi_pins;
volatile int SPI_FLAG;

/************| LED STUFF |***************/
#define BRIGHTNESS_PREAMBLE (0x07)
#define START_OF_FRAME_BYTES 4
#define START_OF_FRAME ((uint32_t)0x00000000)
#define END_OF_FRAME ((uint32_t)0x11111111)
#define NUM_OF_LEDS 144
#define END_OF_FRAME_WORDS (3) // ((NUM_OF_LEDS  - 1) / 16 ) rounded up == 9 
// 1 is start of frame word
// 3 is number of end of frame words == 9 bytes == 3 words
#define COMPLETE_LED_FRAME_SIZE_U32 (NUM_OF_LEDS + 1 + 3)
#define LED_COUNT 144
#define GLOBAL      0
#define RED         3
#define GREEN       2
#define BLUE        1
uint8_t LED_ARRAY[LED_COUNT][4] = {0};
/* 
Uint32_t containing the 4 bytes of the LED frame
    brightness with 3 msp bits 111 and 5 lsb bits birghtness level
    blue
    green
    red
*/
#define LED_FRAME(a,b,g,r) ( (uint32_t)  ( (BRIGHTNESS_PREAMBLE) << 5 | \
                                                    ((uint8_t)a)      | \
                                                    ((uint8_t)b) << 8 | \
                                                    ((uint8_t)g) << 16 | \
                                                    ((uint8_t)r) << 24)| \
                                                    (BRIGHTNESS_PREAMBLE) << 29 )


/***************************************************************/
void rangeSensorTask(void *pvParameters)
{
    uint8_t mCount = 0;
    uint8_t measurment[10];
    memset(measurment, 0x0, 10);
    uint8_t indexLocation = 0;    
    int error;
    //seed random number generator
    srand(42);
    while(1)
    {
       
       // Wait for the semaphore to be given back by Task B
       // xSemaphoreTake(xSemaphore, portMAX_DELAY);

        enableUart();
        error = MXC_UART_Transaction(&read_req);
        if(error != E_NO_ERROR)
        {
            APP_TRACE_INFO1("-->Error with UART_ReadAsync; %d\n", error);
        }
        // while (READ_FLAG){

        // }; 
        

            if (RxData[0] == '\n')
            {
                // mCount++;
                // if (mCount == 2)
                // {
                //     // we have a full measurement
                    mCount = 0;
                    indexLocation-2;
                    measurment[indexLocation] = '\0';
                    measurement_mm = atoi((const char*)measurment);
                    APP_TRACE_INFO1("%u", measurement_mm);
                    indexLocation = 0;
                    disableUart();
                    xSemaphoreGive(xSemaphore);
                //}
            }else{
                    // we have a partial measurement
                    measurment[indexLocation] = RxData[0];
                    indexLocation++;

                }

            READ_FLAG=1;
        
    }
}
void disableUart(void)
{
    NVIC_ClearPendingIRQ(UART3_IRQn);
    NVIC_DisableIRQ(UART3_IRQn);

}
void enableUart(void)
{
    NVIC_ClearPendingIRQ(UART3_IRQn);
    MXC_NVIC_SetVector(UART3_IRQn, UART3_Handler);
    NVIC_EnableIRQ(UART3_IRQn);  
}
/***************************************************************/
void ledStripTask(void *pvParameters)
{
    clearLedArray();
    uint16_t prevMeasurement = 0;
    led_color_t color = {31, 255, 0, 0};
    while(1)
    {
         // Wait for the semaphore to be given back by Task B
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
         __asm volatile("cpsid i");
       if(measurement_mm != prevMeasurement)
       {
           
           prevMeasurement = measurement_mm;
           skSetUnmappedLed(measurement_mm);
           skUpdateLed();
           //fillLEDs(measurement_mm);
       }
         // Give the semaphore back to signal Task A to run
        xSemaphoreGive(xSemaphore);
        __asm volatile("cpsie i");
    }
}
/***************************************************************/
void registerUserAppTasks(void)
{
    // Create the binary semaphore
    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemaphore);
    APP_TRACE_INFO0(">>> Registering user application tasks...");
    xTaskCreate(rangeSensorTask, "Range Sensor Task", 256, NULL, 1, &rangeSensorTask_id);
    xTaskCreate(ledStripTask, "LED Strip Task", 256, NULL, 1, &ledStripTask_id);
}
/***************************************************************/
void UART3_Handler(void)
{
    MXC_UART_AsyncHandler(MXC_UART3);
}
/***************************************************************/
void SPI_IRQHandler(void)
{
    MXC_SPI_AsyncHandler(SPI);
     SPI_FLAG = 0;

}
/***************************************************************/
void SPI_Callback(mxc_spi_req_t *req, int error)
{
   SPI_FLAG = error;
}
/***************************************************************/
void readCallback(mxc_uart_req_t *req, int error)
{
    READ_FLAG = error;
}
/***************************************************************/
int initUART(void)
{
    int error;
    APP_TRACE_INFO0(">>> Initializing UART..."); 
    NVIC_ClearPendingIRQ(UART3_IRQn);
    NVIC_DisableIRQ(UART3_IRQn);
    MXC_NVIC_SetVector(UART3_IRQn, UART3_Handler);
    NVIC_EnableIRQ(UART3_IRQn);  
    // Initialize the UART
    if ((error = MXC_UART_Init(MXC_UART3, RANGE_SENSOR_UART_BAUD, MXC_UART_IBRO_CLK)) != E_NO_ERROR) {
        APP_TRACE_INFO1("-->Error initializing UART: %d\n", error);
        APP_TRACE_INFO0("-->Example Failed\n");
        return error;
    }
    else
    {
        APP_TRACE_INFO0("-->UART Initialized OK\n");
    }
   
    read_req.uart = MXC_UART3;
    read_req.rxData = RxData;
    read_req.rxLen = 1; //we will interrupt at every char recevied, so len == 1
    read_req.txLen = 0;
    read_req.callback = readCallback;
    READ_FLAG = 1;

    return error;
}
/***************************************************************/
int initSpi(void)
{
    int retVal = 0;
    MXC_NVIC_SetVector(SPI_IRQ, SPI_IRQHandler);
    NVIC_EnableIRQ(SPI_IRQ);

    spi_pins.clock = TRUE;
    spi_pins.miso  = TRUE;
    spi_pins.mosi  = TRUE;
    spi_pins.sdio2 = FALSE;
    spi_pins.sdio3 = FALSE;
    spi_pins.ss0   = FALSE;
    spi_pins.ss1   = TRUE;
    spi_pins.ss2   = FALSE;
    spi_pins.vddioh = TRUE;
    
    // Configure the peripheral
    retVal = MXC_SPI_Init(SPI, 1, 0, 1, 0, SPI_SPEED, spi_pins);
    if (retVal != E_NO_ERROR) {
        APP_TRACE_INFO0("> SPI INITIALIZATION ERROR");
        return retVal;
    }

    //SPI Request
    req.spi = SPI;
    req.txData = NULL;
    req.rxData = NULL;
    req.txLen = 0;
    req.rxLen = 0;
    req.ssIdx = 1;
    req.ssDeassert = 1;
    req.txCnt = 0;
    req.rxCnt = 0;
    req.completeCB = (spi_complete_cb_t)SPI_Callback;
    SPI_FLAG = 1;

    retVal = MXC_SPI_SetDataSize(SPI, 8);
   

    if (retVal != E_NO_ERROR) {
        APP_TRACE_INFO1("> SPI SET DATASIZE ERROR: %d", retVal);
        return retVal;
    }

    retVal = MXC_SPI_SetWidth(SPI, SPI_WIDTH_STANDARD);

    if (retVal != E_NO_ERROR) {
        APP_TRACE_INFO1("> SPI SET WIDTH ERROR: %d", retVal);
        return retVal;
    }
    
    return E_NO_ERROR;
}
/***************************************************************/


void spiSend(uint8_t data)
{
    int retVal = 0;
    req.txData = (uint8_t*)&data;
    req.txLen = 1;
    req.txCnt = 0;
    req.rxLen = 0;
    retVal = MXC_SPI_MasterTransaction(&req);

}

void clearLedArray(void)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    LED_ARRAY[i][GLOBAL] = 0xe0;
    LED_ARRAY[i][RED] = 0;
    LED_ARRAY[i][GREEN] = 0;
    LED_ARRAY[i][BLUE] = 0;
  }
}

void skSetLed(uint8_t ledNum, uint8_t global, uint8_t r, uint8_t g , uint8_t b)
{
  LED_ARRAY[ledNum][GLOBAL] = 0xE0 |global;
  LED_ARRAY[ledNum][RED] = r;
  LED_ARRAY[ledNum][GREEN] = g;
  LED_ARRAY[ledNum][BLUE] = b;
}

void skUpdateLed(void)
{
 //   __asm volatile("cpsid i");
  //send start of frame
  for(int i = 0 ; i < 4;i++)
  {
    spiSend(0x00);
  }
  //send led frames
  for(int i = 0 ; i < LED_COUNT;i++)
  {
    spiSend(LED_ARRAY[i][GLOBAL]);
    spiSend(LED_ARRAY[i][BLUE]);
    spiSend(LED_ARRAY[i][GREEN]);
    spiSend(LED_ARRAY[i][RED]);
  }

  //send end of frame
  for(int i = 0 ; i < 4;i++)
  {
    spiSend(0xFF);
  }

 // __asm volatile("cpsie i");
}

void skSetUnmappedLed(uint16_t rangeSensorValue)
{
    static int lastValue = 0;  // This variable will hold the last value between calls

    int ledsToLight = (LED_COUNT - 1) - ((rangeSensorValue * (LED_COUNT - 1)) / 2000);
    APP_TRACE_INFO1("Setting led: %d", ledsToLight);
    //turn old one off
    skSetLed(lastValue,0,0,0,0);
    //turn new one on
    skSetLed(ledsToLight,31,0,0,255);
    lastValue = ledsToLight;
}

void fillLEDs(int value)
{
    __asm volatile("cpsid i");
    static int lastValue = 0;  // This variable will hold the last value between calls
    uint8_t delayUs = 1;
    uint8_t onBrightness = 1;
    uint8_t trailBrightness = 1;
    uint8_t tipBrightness = 10;
    // Map the value from the range 10-2200 to the range 0-143
    //int ledsToLight = ((value - 10) * (LED_COUNT - 1)) / (2200 - 10);
    int ledsToLight = (LED_COUNT - 1) - ((value * (LED_COUNT - 1)) / 2200);
    // Ensure that ledsToLight is within the range 0-143
    if (ledsToLight < 0) ledsToLight = 0;
    if (ledsToLight > LED_COUNT - 1) ledsToLight = LED_COUNT - 1;

    if (ledsToLight < lastValue)  // The new value is smaller, so turn off LEDs from the end
    {
        // turns off all leds one by one that are greater than the new value
        for (int i = lastValue-1; i > ledsToLight; i--)
        {
            // if you want a trailing color set that instead of 0,0,0,
            skSetLed(i, trailBrightness, 0, 0, 0);  // Turn off the LED
            skUpdateLed();
            //MXC_Delay(delayUs);  // Delay to create an animation effect
        }
        // //makes the top led marker drops down to the new value
        // for (int i = lastValue; i >= ledsToLight; i--)
        // {
        //     skSetLed(i, 0, 0, 0, 0);  // Turn off the LED
        //     skSetLed(i - 1,tipBrightness, 255, 255, 0);  // Set the LED just behind the green one to blue
        //     skUpdateLed();
        //    // MXC_Delay(delayUs);  // Delay to create an animation effect
        // }
        if (ledsToLight > 0) {
            skSetLed(ledsToLight , onBrightness, 0, 0, 255);  // Set the LED just behind the green one to blue
            // set tip marker
           // skSetLed(ledsToLight , tipBrightness, 255, 255, 0);  // Set the LED just behind the green one to blue
            skUpdateLed();
        }
    }

    if (ledsToLight > lastValue)  // The new value is larger, so light up additional LEDs
    {
        for (int i = lastValue + 1; i <= ledsToLight; i++)
        {
            skSetLed(i - 1, onBrightness, 0, 0, 255);  // Trailing color (blue in this case)
            //set tip led
          //  skSetLed(i, tipBrightness, 255, 255, 0);  // Primary color (green in this case)
            skUpdateLed();
          //  MXC_Delay(5);  // Delay to create an animation effect
        }
    }

    // Remember the new value for the next time the function is called
    lastValue = ledsToLight;
     __asm volatile("cpsie i");
}

