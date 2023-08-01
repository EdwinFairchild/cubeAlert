#include "user_app.h"

/************| FreeRTOS STUFF |***************/
TaskHandle_t rangeSensorTask_id;
TaskHandle_t ledStripTask_id;

/************| UART STUFF |***************/
#define RANGE_SENSOR_UART_BAUD 9600
volatile int READ_FLAG;
uint8_t RxData[1]; // we wil only be using 1 space but just to be safe make it bigger
 mxc_uart_req_t read_req;
uint16_t measurement_mm = 0;

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
#define SPI_SPEED 100000 // Bit Rate
mxc_spi_req_t req;
mxc_spi_pins_t spi_pins;
volatile int SPI_FLAG;

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


/***************************************************************/
void rangeSensorTask(void *pvParameters)
{
    uint8_t mCount = 0;
    uint8_t measurment[10];
    memset(measurment, 0x0, 10);
    uint8_t indexLocation = 0;    
    int error;
    while(1)
    {
       
        error = MXC_UART_TransactionAsync(&read_req);
        if(error != E_NO_ERROR)
        {
            APP_TRACE_INFO1("-->Error with UART_ReadAsync; %d\n", error);
        }
        while (READ_FLAG){}; 
        
        if (READ_FLAG != E_NO_ERROR) {
            APP_TRACE_INFO1("-->Error with UART_ReadAsync callback; %d\n", READ_FLAG);
        }
        else
        {
            // check and update the times we encounter 'm' as RxData
            if (RxData[0] == 'm')
            {
                mCount++;
                if (mCount == 2)
                {
                    // we have a full measurement
                    mCount = 0;
                    indexLocation++;
                    measurment[indexLocation] = '\0';
                    measurement_mm = atoi((const char*)measurment);
                    APP_TRACE_INFO1("%u", measurement_mm);
                    indexLocation = 0;
                }
            }else{
                    // we have a partial measurement
                    measurment[indexLocation] = RxData[0];
                    indexLocation++;

                }

            READ_FLAG=1;
        }
    }
}
/***************************************************************/
void ledStripTask(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(1000);
        APP_TRACE_INFO0("LED Strip Task");
    }
}
/***************************************************************/
void registerUserAppTasks(void)
{
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
    spi_pins.clock = TRUE;
    spi_pins.miso  = TRUE;
    spi_pins.mosi  = TRUE;
    spi_pins.sdio2 = FALSE;
    spi_pins.sdio3 = FALSE;
    spi_pins.ss0   = FALSE;
    spi_pins.ss1   = TRUE;
    spi_pins.ss2   = FALSE;
    // Configure the peripheral
    retVal = MXC_SPI_Init(SPI, 1, 0, 1, 0, SPI_SPEED, spi_pins);
    if (retVal != E_NO_ERROR) {
        APP_TRACE_INFO0("> SPI INITIALIZATION ERROR");
        return retVal;
    }

    //SPI Request
    req.spi = SPI;
    req.txData = NULL;
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
void spiSendLedStripFrame(void)
{
    MXC_NVIC_SetVector(SPI_IRQ, SPI_IRQHandler);
        NVIC_EnableIRQ(SPI_IRQ);


    MXC_SPI_MasterTransactionAsync(&req);

    while (SPI_FLAG == 1) {}
}