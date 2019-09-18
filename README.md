## Vesc UART implementation on a STM32F4 Discovery board.

This project implements a communication between a host mcu (STM32F4-disco) and a VESC using UART. The code is using the STM32 Stdperiph library as well as the USART and VCP library by:  
https://stm32f4-discovery.net

A virtuall COM port is implemented and user can poll data from the VESC via this. 
