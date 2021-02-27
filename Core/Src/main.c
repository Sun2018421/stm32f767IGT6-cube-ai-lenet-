/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "myiic.h"
#include "pcf8574.h"
#include "ov2640.h"
#include "dcmi.h"
#include "app_x-cube-ai.h"
#include <stdio.h>


CRC_HandleTypeDef hcrc;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);

#define output_width 28
#define output_height 28


u8 Inputdatamode = 1; /*0代表从串口输入
												1代表为从摄像头输入
											*/
u16 dcmi_line_buf[output_width];  //一行空间缓冲
u16 picture_data_buf[output_width*output_height]; //整个一张图片缓冲

volatile u8 currentline = 0 ;
volatile u8 one_shot_ok = 0;

void jpeg_data_process(void)
{
	one_shot_ok = 1;
	currentline = 0;
}

void DCMI_rx_callback(void){
	u16 * datapointer ;
	u16 *pbuf;
	u16 i ;
	pbuf=(u16*)dcmi_line_buf; 

	datapointer = (u16*)picture_data_buf;
	datapointer = datapointer + currentline*output_width;
	if(one_shot_ok == 0){
		for(i=0; i <output_width ; i++){
			 datapointer[i]= pbuf[i];
		}
	}
	if(currentline < output_height) currentline++;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	u8 i , j;					
	u8 fileout = 1;
	u16 *ouputpointer;
  SCB_EnableICache();
  SCB_EnableDCache();
  HAL_Init();
	//	SystemClock_Config(); 修改
	Stm32_Clock_Init(432,25,2,9);
	uart_init(115200);
	delay_init(216);
	MX_GPIO_Init();  //相关配置初始化
	MX_CRC_Init();
  MX_X_CUBE_AI_Init();

	if(Inputdatamode == 1){
		PCF8574_Init();
		while(OV2640_Init()){
		}
		DCMI_Init();
		OV2640_RGB565_Mode();	//RGB565模式 
		OV2640_Light_Mode(0);	//自动模式
		OV2640_Color_Saturation(3);//色彩饱和度0
		OV2640_Brightness(4);	//亮度0
		OV2640_Contrast(3);		//对比度0
		//OV2640_Special_Effects(2); //设置成黑白
		OV2640_ImageWin_Set(0,0,800,600); // 800 = 40 * 20 , 576 = 32 *18
		OV2640_OutSize_Set(output_width,output_height); //缩放
		dcmi_rx_callback = DCMI_rx_callback;   //设置回调函数	
		DCMI_DMA_Init((u32)dcmi_line_buf,0,output_width/2,DMA_MDATAALIGN_HALFWORD,DMA_MINC_ENABLE);
		currentline = 0 ;
		one_shot_ok = 0 ;		
		DCMI_Start();
		while(1){
			if((one_shot_ok == 1) && (fileout==1)){
			ouputpointer = (u16*)picture_data_buf;
				for(i=0;i < output_height ; i++){
						for(j=0 ; j < output_width ; j++){
							printf("%x ",ouputpointer[i*output_width+j]);
						}
				printf("\r\n");
				}
				fileout = 0;
			}
			if(fileout == 0){
				break;
			}
		}
	}
	MX_X_CUBE_AI_Process();
}


/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
//int fputc(int ch, FILE *f)
//{ 	
//	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕   
//	USART1->TDR=(unsigned char)ch;      
//	return ch;
//}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
