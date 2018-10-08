#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <MQTTClient.h>
#include "double_conversion.h"
#include "pb.h"	
#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "sitewhere_pb.h"
#include "sitewhere.h"
               
#define ORIGINATOR			      "Basic YWRtaW46cGFzc3dvcmQ"			       
#define TOPIC 				      "SiteWhere/input/protobuf"				    
#define QOS         				1
#define TIMEOUT_MS     				10000L
#define CONNECT_TIMEOUT_SECOND			3
#define TRUE        				1
#define FALSE       				0
#define STDERR      				2
#define MAXBUFF						512
#define TOPIC_LEN					100
#define DEFAULT_INITIALIZE			0
#define DEFALUT_EVENT_TIME			FALSE
#define STATUS_SUCCESS  			0
#define STATUS_FAILURE 				(-1)
#define ERR_CONNECT					(-2)
#define ERR_SUBSCRIBE				(-3)
#define CON_LOST					(-4)
#define ERR_PUBLISH					(-5)

short ConnectDevice(MQTTClient*,char*,char*,char*);
short RegisterDevice(MQTTClient,char*,char*);
short SendMeasurement(MQTTClient,char*,char*,float);
short SendLocation(MQTTClient,char*,float,float,float);
short SendAlert(MQTTClient,char*,char*,char*);
void DisconnectDevice(MQTTClient);
