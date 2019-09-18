/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  *
  *
  *
  * This code is built upon the VCP librarie from @author  Tilen Majerle:
  * https://stm32f4-discovery.net/2014/08/library-24-virtual-com-port-vcp-stm32f4xx/
  *
  * It also includes the USART,gpio, and disco lib from the same author.
  *
  *
  * It also includes the BLDC interface library from vedder:
  * http://vedder.se/2015/10/communicating-with-the-vesc-using-uart/
  * https://github.com/vedderb/bldc_uart_comm_stm32f4_discovery
  *
  *
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

#include "main.h"
#include "defines.h"
#include "tm_stm32f4_usb_vcp.h"
#include "tm_stm32f4_disco.h"
			
#include "tm_stm32f4_usart.h"

void TIM3_init(void);


char string_buffer[300];

float motor_pos = 0.0f;
float current_in = 0.0f;

static void bldc_send_packet(unsigned char *data, unsigned int len) {

	// Your implementation
	TM_USART_Send(USART1, data, len);

}

void bldc_val_received(mc_values *val) {

	motor_pos = val->pid_pos;
	current_in = val->current_in;

	sprintf(string_buffer, "Input voltage: %.2f V\r\n"
			"Temp:          %.2f degC\r\n"
			"Current motor: %.2f A\r\n"
			"Current in:    %.2f A\r\n"
			"RPM:           %.1f RPM\r\n"
			"PID Pos:       %.1f Deg\r\n"
			"Duty cycle:    %.1f %%\r\n"
			"Fault Code:    %s\r\n", val->v_in, val->temp_mos, val->current_motor, val->current_in, val->rpm, motor_pos, val->duty_now * 100.0, bldc_interface_fault_to_string(val->fault_code));
	TM_USB_VCP_Puts(string_buffer);
}

int main(void)
{
		RCC_ClocksTypeDef RCC_Clocks;
		uint8_t c;

	    /* System Init */
	    SystemInit();
	    RCC_GetClocksFreq(&RCC_Clocks);


	    /* Initialize LED's. Make sure to check settings for your board in tm_stm32f4_disco.h file */
	    TM_DISCO_LedInit();

	    TIM3_init();

	    /* Initialize USB VCP */
	    TM_USB_VCP_Init();

		/* Initialize USART1 at 9600 baud, TX: PB6, RX: PB7 */
		TM_USART_Init(USART1, TM_USART_PinsPack_2, 115200);

		// Initialize the bldc interface and provide the send function
		bldc_interface_uart_init(bldc_send_packet);
		bldc_interface_set_rx_value_func(bldc_val_received);


	    while (1) {
	        /* USB configured OK, drivers OK */
	        if (TM_USB_VCP_GetStatus() == TM_USB_VCP_CONNECTED) {
	            /* Turn on GREEN led */
	            TM_DISCO_LedOn(LED_GREEN);
	            TM_DISCO_LedOff(LED_RED);
	            /* If something arrived at VCP */
	            if (TM_USB_VCP_Getc(&c) == TM_USB_VCP_DATA_OK) {
	                /* Return data back */
	                //TM_USB_VCP_Putc(c);
	            	//TM_USART_Puts(USART1, &c);
	            	bldc_interface_get_values();

	            }
	        } else {
	            /* USB not OK */
	            TM_DISCO_LedOff(LED_GREEN);
	            TM_DISCO_LedOn(LED_RED);
	        }
	}
}

void TM_USART1_ReceiveHandler(uint8_t c) {
   //Do your stuff here when byte is received
	bldc_interface_uart_process_byte(c);
}


void TIM3_init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  __IO uint16_t timer_period = 500;
  uint16_t PrescalerValue = 0;

  /* -----------------------------------------------------------------------
    TIM3 Configuration: Output Compare Timing Mode:

    In this example TIM3 input clock (TIM3CLK) is set to 2 * APB1 clock (PCLK1),
    since APB1 prescaler is different from 1.
      TIM3CLK = 2 * PCLK1
      PCLK1 = HCLK / 4
      => TIM3CLK = HCLK / 2 = SystemCoreClock /2

    To get TIM3 counter clock at 0.5 MHz, the prescaler is computed as follows:
       Prescaler = (TIM3CLK / TIM3 counter clock) - 1
       Prescaler = ((SystemCoreClock /2) /0.5 MHz) - 1

    timer update rate = TIM3 counter clock / timer_period = 1000 Hz
    -->> timer_period = 500

    Note:
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
     Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
     function to update SystemCoreClock variable value. Otherwise, any configuration
     based on this variable will be incorrect.
  ----------------------------------------------------------------------- */

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 500000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = timer_period;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);

  /* TIM Interrupts enable */
  TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);

}


