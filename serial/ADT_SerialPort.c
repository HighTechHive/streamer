#include "ADT_SerialPort.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ADT_SerialPortStruct* dummyStruct;

void ADT_initSerialPort(ADT_SerialPortStruct *serialPortInfo)
{
	serialPortInfo->buffer = (unsigned char*)malloc(sizeof(char)*BUFFERSIZE);
	//dummyStruct = serialPortInfo;
	strcpy((char*)serialPortInfo->buffer, "DEFAULT MESSAGE");
	// printf("%s\n",serialPortInfo->deviceName );
	if ((serialPortInfo->fileDescriptor = open(serialPortInfo->deviceName, O_RDWR | O_NONBLOCK | O_NOCTTY ) ) < 0)
	{
		printf("could not open: %s \n",serialPortInfo->deviceName);
		exit(1);
	}
	else
	{
		fcntl(serialPortInfo->fileDescriptor, F_SETFL, 0);
		serialPortInfo->channel = g_io_channel_unix_new (serialPortInfo->fileDescriptor);
		g_io_add_watch(serialPortInfo->channel, G_IO_IN, &ttycallback, serialPortInfo);
		ADT_config(serialPortInfo);
		printf("port %s is now open\n", serialPortInfo->deviceName);
	}
} 	

//------------------------------------------------------------------------------

// End of the stream
void eos_event_handler(int dummy) {
    printf("The user finish the stream!\n");
    // Spread the end of stream event
    close(dummyStruct->fileDescriptor);
    //g_main_loop_quit(dummyStruct->mainLoop);
}

//------------------------------------------------------------------------------


gboolean ttycallback(GIOChannel *source, GIOCondition condition, void *data)
{
	//printf("ttycallback\n");
	//printf("%s\n", ((ADT_SerialPortStruct*)data)->deviceName);
	((ADT_SerialPortStruct*)data)->bufferLength=read(((ADT_SerialPortStruct*)data)->fileDescriptor,((ADT_SerialPortStruct*)data)->buffer, BUFFERSIZE);
    //onGetData(((ADT_SerialPortStruct*)data)->bufferLength, ((ADT_SerialPortStruct*)data)->buffer);
	return 1;
}


//------------------------------------------------------------------------------

//	call back function activated when data is recived
// void onGetData(unsigned int bufferLength, unsigned char* buffer)
// {	
// 	printf("onGetData()\n");
// 	printf("Message (%d)\n", bufferLength);

// 	printf("MessageHEX (%d): ", bufferLength);
// 	for(unsigned int i=0; i<bufferLength; i++){
// 		printf("<%02x> ", buffer[i]);
// 	}
// 	printf("\n");
// 	for(unsigned int i=0; i<bufferLength; i++){
// 		printf("%c",buffer[i]);
// 	}
// 	printf("\n");
// }

//------------------------------------------------------------------------------
int ADT_config(ADT_SerialPortStruct *serialPortInfo)
{
	struct termios port_settings;      // structure to store the port settings in

   	tcgetattr(serialPortInfo->fileDescriptor, &port_settings);

	speed_t portSpeed = B9600;
	
	switch(serialPortInfo->speed)
	{
		case 0:
			portSpeed=B0;
			break;
		case 50:
			portSpeed=B50;
			break;
		case 75:
			portSpeed=B75;
			break;
		case 110:
			portSpeed=B110;
			break;
		case 134:
			portSpeed=B134;
			break;
		case 150:
			portSpeed=B150;
			break;
		case 200:
			portSpeed=B200;
			break;
		case 300:
			portSpeed=B300;
			break;
		case 600:
			portSpeed=B600;
			break;
		case 1200:
			portSpeed=B1200;
			break;
		case 1800:
			portSpeed=B1800;
			break;
		case 2400:
			portSpeed=B2400;
			break;
		case 4800:
			portSpeed=B4800;
			break;
		case 9600:
			portSpeed=B9600;
			break;
		case 19200:
			portSpeed=B19200;
			break;
		case 38400:
			portSpeed=B38400;
			break;	
		case 115200:
			portSpeed=B115200;
			break;	
		default:
			portSpeed=B0;
			break;
	}

	cfsetispeed(&port_settings, portSpeed);    // set baud rates
	cfsetospeed(&port_settings, portSpeed);
	
	port_settings.c_cflag &= ~CSIZE;	/* Clear current char size mask */
	port_settings.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
	port_settings.c_cflag &= ~CSTOPB;
	port_settings.c_cflag &= ~ECHO;
	//port_settings.c_cflag |= CRTSCTS; 
		
	switch(serialPortInfo->settings[0])
	{
		case '5':
			port_settings.c_cflag |= CS5;    /* Select 5 data bits */
			break;
		case '6':
			port_settings.c_cflag |= CS6;    /* Select 6 data bits */
			break;
		case '7':
			port_settings.c_cflag |= CS7;    /* Select 7 data bits */
			break;
		case '8':
			port_settings.c_cflag |= CS8;    /* Select 8 data bits */
			break;
		default:
			port_settings.c_cflag |= CS8;    /* Select 8 data bits */
			break;
	}
	switch(serialPortInfo->settings[1])
	{
		case 'E':
			port_settings.c_cflag |= PARENB;    /* Enable parity */
			break;
		case 'N':
			port_settings.c_cflag &= ~PARENB;    /* Diable parity */
			break;
		default:
			port_settings.c_cflag &= ~PARENB;    /* Diable parity */
			break;
	}
	switch(serialPortInfo->settings[2])
	{
		case '1':
			port_settings.c_cflag &= ~CSTOPB;    /* Set one stop bit */
			break;
		case '2':
			port_settings.c_cflag |= CSTOPB;    /* Set two stop bits */
			break;
		default:
			port_settings.c_cflag |= CSTOPB;    /* Set two stop bits */
			break;
	}
	tcflush(serialPortInfo->fileDescriptor, TCIFLUSH);
	tcsetattr(serialPortInfo->fileDescriptor, TCSANOW, &port_settings);    // apply the settings to the port
	return(serialPortInfo->fileDescriptor);
}