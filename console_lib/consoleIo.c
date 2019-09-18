// Console IO is a wrapper between the actual in and output and the console code
// In an embedded system, this might interface to a UART driver.

#include "consoleIo.h"
#include <stdio.h>
#include "main.h"


eConsoleError ConsoleIoInit(void)
{
	return CONSOLE_SUCCESS;
}
eConsoleError ConsoleIoReceive(uint8_t *buffer, const uint32_t bufferLength, uint32_t *readLength)
{
	uint8_t i = 0;
	
	//return 0 if nothing
	//Return number of characters in string otherwise
	i = TM_USB_VCP_Gets((char*) buffer, (uint16_t) bufferLength);

	*readLength = i;
	return CONSOLE_SUCCESS;
}

eConsoleError ConsoleIoSend(const uint8_t *buffer, const uint32_t bufferLength, uint32_t *sentLength)
{
	TM_USB_VCP_Send((uint8_t*)buffer, bufferLength);
	*sentLength = bufferLength;
	return CONSOLE_SUCCESS;
}

eConsoleError ConsoleIoSendString(const char *buffer)
{
	//Print over VCP
	TM_USB_VCP_Puts((char*)buffer);
	return CONSOLE_SUCCESS;
}

