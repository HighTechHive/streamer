#ifndef ADT_SERIALPORT_H
#define ADT_SERIALPORT_H

#include <glib.h>
#include <sys/stat.h>
#include <fcntl.h> // File control definitions
#include <unistd.h>
#include <termios.h> // POSIX terminal control definitionss

#define	BUFFERSIZE	255

typedef struct _ADT_SerialPort	ADT_SerialPortStruct;

struct _ADT_SerialPort
{
	const char* deviceName;
	const char* settings;
	int speed;
	int fileDescriptor;
	GIOChannel* channel;
	unsigned char* buffer;
	unsigned int bufferLength;
	GMainLoop* mainLoop;
};

void ADT_initSerialPort(ADT_SerialPortStruct *serialPortInfo);
int ADT_config(ADT_SerialPortStruct *serialPortInfo);
void eos_event_handler(int dummy);
gboolean ttycallback(GIOChannel *source, GIOCondition condition, void *data);
//void onGetData(unsigned int bufferLength, unsigned char* buffer);

#endif /* ADT_SERIALPORT_H */