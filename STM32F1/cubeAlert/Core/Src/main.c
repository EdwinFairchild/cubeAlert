/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "stm32f1xx.h"
//import lib needed to use srand and rand
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED_COUNT 144
#define GLOBAL      0
#define RED         3
#define GREEN       2
#define BLUE        1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t LED_ARRAY[144][4] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void CL_printMsg(char *msg, ...)
{	
	char buff[80];	
	va_list args;
	va_start(args, msg);
	vsprintf(buff, msg, args);
		
	for (int i = 0; i < strlen(buff); i++)
	{		
		USART1->DR = buff[i];
		while( !( USART1->SR & USART_SR_TXE )  );
	}		
		
	while (!(USART1->SR & USART_SR_TC));		
}

void spiSend(uint8_t data)
{
  
  SPI1->DR = data;
  while (!(SPI1->SR & SPI_SR_TXE));
}

void clearLedArray(void)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    LED_ARRAY[i][GLOBAL] = 0xE0;
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
}
void fillLEDs(int value)
{
    // Map the value from the range 10-2200 to the range 0-143
    int ledsToLight = ((value - 10) * (LED_COUNT - 1)) / (2200 - 10);

    // Ensure that ledsToLight is within the range 0-143
    if (ledsToLight < 0) ledsToLight = 0;
    if (ledsToLight > LED_COUNT - 1) ledsToLight = LED_COUNT - 1;

    // Clear all LEDs
    clearLedArray();

    // Light up the LEDs from 0 to ledsToLight
    for (int i = 0; i <= ledsToLight; i++)
    {
        skSetLed(i, 1, 0, 255, 0);  // You can choose any color here
    }

    // Update the LEDs
    skUpdateLed();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //seed rand
  srand(42);
  //enable spi
  SPI1->CR1 |= SPI_CR1_SPE; 
  //delay for 1 second
  HAL_Delay(1000);
  clearLedArray();
  skSetLed(0,10,255,0,0);
  skSetLed(1,10,0,0x42,0);
  skSetLed(2,10,0,0,0x66);
  skUpdateLed();
  
 
  uint8_t num = 0;
  // Initialize LED position
      // Initialize LED position and movement direction
    int ledPos = 0;
    int bounceCount = 0;
    int bounceDistance = LED_COUNT / 4;  // Set initial bounce distance to a quarter of the strip length
    int direction = 1;
    fillLEDs(2200);
    while (1)
    {
      CL_printMsg("Hello World %d \r\n",num);
        //toggle PC13
        // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // //move led along the strip, with easing effect towards the end
        // clearLedArray();
        // skSetLed(ledPos, 10, 0, 255, 0);  // You can choose any color here
        // skUpdateLed();
        // HAL_Delay(1 + (ledPos * ledPos) / (10*LED_COUNT));  // delay longer as we get closer to the end
        
        // // Increment or decrement LED position depending on direction
        // ledPos += direction;
        // if (ledPos >= LED_COUNT) {  // If we have reached the end of the strip...
        //     ledPos = LED_COUNT - 1;  // ...stay at the end,
        //     direction = -1;  // ...and reverse direction for bounce
        //     bounceCount++;
        // }
        // else if (ledPos <= LED_COUNT - 1 - bounceDistance && direction == -1) {  // If we have reached the bounce point...
        //     direction = 1;  // ...reverse direction for bounce
        //     bounceDistance /= 2;  // Halve the bounce distance for next time
        // }

        // if (bounceCount > 3) {  // If we have bounced enough times...
        //     break;  // ...exit the loop
        // }
    }
}
void bounceLED(void)
{
    // Initialize LED position and movement direction
    int ledPos = 0;
    int direction = 1;

    while (1)
    {
        //toggle PC13
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        //move led along the strip, with easing effect towards the end
        clearLedArray();
        skSetLed(ledPos, 10, 0, 255, 0);  // You can choose any color here
        skUpdateLed();
        HAL_Delay(10 + (ledPos * ledPos) / (LED_COUNT));  // delay longer as we get closer to the end
        
        // Increment or decrement LED position depending on direction
        ledPos += direction;
        if (ledPos >= LED_COUNT) {  // If we have reached the end of the strip...
            ledPos = LED_COUNT - 2;  // ...move one step back,
            direction = -1;  // ...and reverse direction
        }
        else if (ledPos < 0) {  // If we have reached the start of the strip...
            ledPos = 1;  // ...move one step forward,
            direction = 1;  // ...and reverse direction
        }
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
