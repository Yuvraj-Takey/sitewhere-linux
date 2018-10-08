#include "sw_lib/sw_bridge.h"
#include <unistd.h>
#define STATIC	10

// create the client object
static MQTTClient client;
// signal handler
void InvalidTermination(int);

int main(int argc, char const *argv[])
{
	// registering signal and its handlers
	signal(SIGINT, InvalidTermination);    // ctrl+c
	signal(SIGTSTP, InvalidTermination);   // ctrl+z

	// Information
	short iRet_Connect = 0, s_index = 0, iRet_RegisDev = 0;
	char address[] = "127.0.0.1:1883";
	char clientID[] = "Axonet-Emsys";
	char HardWareID[] = "IoT-Linux_Gateway";
	char specification[] = "7dfd6d63-5e8d-4380-be04-fc5c73801dfb";
	char Data[] = "Temperature";
	float value = 32.865;
	float latitude = 2.154;
	float longitude = 5.161;
	float elevation = 520;
	char alertType[] = "INFO";
	char alertMessage[] = "SW_Linux";

	   
	// connect to the sitewhere network
	if((iRet_Connect = ConnectDevice(&client,address,HardWareID,clientID)) == STATUS_SUCCESS)
	{
		// register our device
	    if((iRet_RegisDev = RegisterDevice(client,HardWareID,specification)) == STATUS_SUCCESS)
	    {
	    	for(s_index = 0; s_index < STATIC; s_index++)
	    	{
	    		// send messages to sitewhere
		        SendMeasurement(client,HardWareID, Data, value);
		        SendLocation(client,HardWareID,latitude,longitude,elevation);
		        SendAlert(client,HardWareID,alertType,alertMessage);
		        sleep(10);
		    }
	    }
	}

	// disconnect the connection
	DisconnectDevice(client);
	  
	return 0;
}

// signal handler
void InvalidTermination(int signum)
{
	// disconnect the connection
	DisconnectDevice(client);
	exit(0);
}
