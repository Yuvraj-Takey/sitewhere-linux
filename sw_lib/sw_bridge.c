#include "sw_bridge.h"

volatile MQTTClient_deliveryToken deliveredtoken;	// Global variable
unsigned short iGet_Connect;						// connection count 

// CALLBACK FUNCTIONS  ============================================================================================================

/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: handleSystemCommand
    Purpose				: Receives acknowledgement for device registration
    Parameters Passed	: 1) char* -Pointer to byte array to read from.
    					  2) uint8_t		-Size of the byte array.
    Return Parameters	: none

    @author Yuvraj Takey
    @version 1.1 18/4/18
---------------------------------------------------------------------------------------------------------------*/
static void handleSystemCommand(char* payload, unsigned int length) {
  Device_Header header;
  Device_RegistrationAck ack = Device_RegistrationAck_init_zero;

/*-------------------------------------------------------------------
 *
 * pb_istream_from_buffer : Helper function for creating an input 
 * stream that reads data from a memory buffer.
 * https://jpa.kapsi.fi/nanopb/docs/reference.html#pb-istream-from-buffer
 *
 *------------------------------------------------------------------*/
  pb_istream_t stream = pb_istream_from_buffer(payload, length);

  // Read header to find what type of command follows.
  if (pb_decode_delimited(&stream, Device_Header_fields, &header)) {
  
    // Handle a registration acknowledgement.
    if (header.command == Device_Command_ACK_REGISTRATION) {
      if (pb_decode_delimited(&stream, Device_RegistrationAck_fields, &ack))
      {	
        if (ack.state == (Device_RegistrationAckState_ALREADY_REGISTERED))
          printf("[DONE]: Device was already registered.\n");
        else if (ack.state == (Device_RegistrationAckState_NEW_REGISTRATION))
          printf("[DONE]: Registered new device.\n");
        else if (ack.state == (Device_RegistrationAckState_REGISTRATION_ERROR))
          dprintf(STDERR,"[ERROR]: registering device.\n");
        else
        	dprintf(STDERR,"[SORRY]: unable to get acknowledgement\nReason: %s\n",strerror(errno));
      }
      else{
        dprintf(STDERR,"[SORRY]: Decoding failed: %s\n", PB_GET_ERROR(&stream));
      }
    }
  } 
  else {
    printf("[SORRY]: unable to decode system command\n");
  }
  printf("====================================================================================\n");
}


/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: delivered (CALLBACK)
    Purpose				: It is called by the client library after the client application has published
    					  a message to the server
    Parameters Passed	: 1) void* 		-A pointer to the context value originally passed to MQTTClient_setCallbacks(),
    									 which contains any application-specific context
    					  2) MQTTClient_deliveryToken		- Applications can check that all messages have been correctly
    					  published by matching the delivery tokens
    Return Parameters	: none

    @author Yuvraj Takey
    @version 1.1 18/4/18
---------------------------------------------------------------------------------------------------------------*/
void delivered(void *context, MQTTClient_deliveryToken d_token)
{
  time_t t;
  time(&t);
  printf("[ACK_]: Message with token value %d delivery confirmed at %s", d_token, ctime(&t));
  deliveredtoken = d_token;
}

/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: msgarrvd (CALLBACK)
    Purpose				: It is called by the client library when a new message that matches a client
    					  subscription has been received from the server. This function is executed on a separate
    					  thread to the one on which the client application is running. 
    Parameters Passed	: 1) void* 		-A pointer to the context value originally passed to MQTTClient_setCallbacks(),
    									 which contains any application-specific context
    					  2) char		-The topic associated with the received message. 
    					  3) int 		-The length of the topic 
    					  4) MQTTClient_message 	-This structure contains the message payload and attributes. 
    Return Parameters	: int 			-Returning true indicates that the message has been successfully handled
    									 by client

    @author Yuvraj Takey
    @version 1.1 18/4/18
-------------------------------------------------------------------------------------------------------------*/
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
  char* cMessage = (char*)malloc(strlen(message->payload));

  printf("Message arrived\n");
  printf("\ttopic:\t%s\n",topicName);
  memcpy(cMessage,message->payload,strlen(message->payload));
  printf("\tmessage: %s\n",cMessage);

  // check the status
  handleSystemCommand(message->payload, message->payloadlen);

  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);

  free(cMessage);
  return 1;
}

/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: connlost (CALLBACK)
    Purpose				: if the client loses its connection to the server this CALLBACK function gets call.
    Parameters Passed	: 1) void* 			-A pointer to the context value originally passed to 
    										 MQTTClient_setCallbacks(),which contains any application-specific context. 
    					  2) char*			-The reason for the disconnection. Currently, cause is always set to NULL. 
    Return Parameters	: none

    @author Yuvraj Takey
    @version 1.1 18/4/18
---------------------------------------------------------------------------------------------------------------*/
void connlost(void *context, char *cause)
{
    dprintf(STDERR,"\nConnection lost \n%s\n",cause);
}


/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: ConnectDevice
    Purpose				: This function create and set-up connection with SiteWhere using MQTT library
    Parameters Passed	: 1) MQTTClient*	-A pointer to an MQTTClient handle. The handle is populated with a
    										 valid client reference following a successful return from this function. 
    					  2) char*			-A null-terminated string specifying the server to which the client
    					  					 will connect(IP:PORT).
    					  3) char*			-Hardware(Device_Name) that you want to subscribe with SiteWhere.
    					  4) char*			-The client identifier passed to the server when the client connects to it.
    					  5) short			-flag for successfull handshaking between Client and Server
    Return Parameters	: short				-describe the flow of execution.

    @author Yuvraj Takey
    @version 1.1 18/4/18
---------------------------------------------------------------------------------------------------------------*/
short ConnectDevice(MQTTClient *client, char* address, char* HardWareID, char* clientID)
{
  short STATUS = DEFAULT_INITIALIZE, iRet_Val = DEFAULT_INITIALIZE;
  char Topic_Sub_Cmd[TOPIC_LEN] = "SiteWhere/commands/";
  char Topic_Sub_Sys[TOPIC_LEN] = "SiteWhere/system/";
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;  // structure : MQTTClient_connectOptions details

  printf("\n*****Welcome To Sitewhere Data Sending Application******\n\n");

  // Get the Client handle(descripter) in &client
  if((iRet_Val = MQTTClient_create(client, address, clientID,MQTTCLIENT_PERSISTENCE_DEFAULT, NULL)) == MQTTCLIENT_SUCCESS)
  {
    printf("\tCreating Connection...\n");
    printf("====================================================================================\n");

    // this will return async notification to CALLBACK fnc
    MQTTClient_setCallbacks(*client, NULL, connlost, msgarrvd, delivered);   
   
    /*---------------------------------------------------------
     *
     * Now,
     *   make peer-to-peer Connection with MQTT Client
     *    as well as subscribes Topic to MQTT Client.
     *
     *  @updated: April 2018
     *------------------------------------------------------*/  

    strcat(Topic_Sub_Cmd,HardWareID);
    strcat(Topic_Sub_Sys,HardWareID);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = TRUE;
    conn_opts.connectTimeout = CONNECT_TIMEOUT_SECOND;
    conn_opts.username = "admin";     // if necessary
    conn_opts.password = "password";  // if necessary

        
     // Connect to the Server AWS(SiteWhere)
    if ((iRet_Val = MQTTClient_connect(*client, &conn_opts)) == MQTTCLIENT_SUCCESS)
    {   
      // This function attempts to subscribe a client to a single topic
      if((iRet_Val = MQTTClient_subscribe(*client, Topic_Sub_Cmd, QOS)) == MQTTCLIENT_SUCCESS)  //TOPIC
      {
        if((iRet_Val = MQTTClient_subscribe(*client, Topic_Sub_Sys, QOS)) == MQTTCLIENT_SUCCESS)  //TOPIC SYS is required for return ack.
        {
          printf("[DONE]: Connected and Subscribed with SiteWhere\n");
          STATUS = STATUS_SUCCESS;
          iGet_Connect++;			// increase connection count
        }
        else
        {
          dprintf(STDERR,"[SORRY]: unable to subscribe the message\nReason : %s",strerror(errno));
          STATUS = (ERR_SUBSCRIBE);
        }
      }
      else
      {
        dprintf(STDERR,"[SORRY]: unable to subscribe the message\nReason : %s",strerror(errno));
        STATUS = (ERR_SUBSCRIBE);
      }
    }
    else
    {
      dprintf(STDERR,"[SORRY]: unable to connect\nReason: %s\n",strerror(errno));
      STATUS = (ERR_CONNECT);
    }
  }
  else
  {
    dprintf(STDERR,"[SORRY]: unable to initiate connection\nReason: %s\n",strerror(errno));
    STATUS = (STATUS_FAILURE);
  }

  return STATUS;
}


/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: RegisterDevice
    Purpose				: This function registers requested device with SiteWhere.
    Parameters Passed	: 1) MQTTClient*	-A pointer to an MQTTClient handle. The handle is populated with a
    										 valid client reference following a successful return from this function. 
    					  2) char*			-Device_Name that you want to register with SiteWhere hardware.
    					  3) char*			-a specification token to the system which in turns creates a new 
    					  					 device record that can start accepting events, this is used to describe 
    					  					 the hardware information/configuration for a type of device.
    Return Parameters	: short				-describe the flow of execution.

    @author Yuvraj Takey
    @version 1.2 19/4/18
---------------------------------------------------------------------------------------------------------------*/
short RegisterDevice(MQTTClient client,char* HardWareID, char* specification)
{
  short iRet_Val = DEFAULT_INITIALIZE;
  uint8_t cData_Buffer[MAXBUFF];
  MQTTClient_deliveryToken token;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;

  // SiteWhere provided method for device-registration
  // This function genarate/fill a cData_Buffer in such a way that we can pass this cData_Buffer to MQTTClient_publishMessage
  if (iRet_Val = sw_register(HardWareID, specification, cData_Buffer, sizeof(cData_Buffer), ORIGINATOR))  
	  {
	    pubmsg.payload = cData_Buffer;              // A pointer to the payload of the MQTT message
	    pubmsg.payloadlen = sizeof(cData_Buffer);   // Length of the Message 
	    pubmsg.qos = QOS;                     		// the message will be delivered more than once in some circumstances
	    pubmsg.retained = FALSE;

	    //This function attempts to publish a message to a given topic(MQTT)
	    if((iRet_Val = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) == MQTTCLIENT_SUCCESS)
	    {
	    	//This function is called by the client application to synchronize execution of the 
          	// main thread with completed publication of a message
	        if((iRet_Val = MQTTClient_waitForCompletion(client, token, TIMEOUT_MS)) == MQTTCLIENT_SUCCESS)   // wait till publishing not completes
	        {
	            printf("[DONE]: Successfully registered with token : %d\n",token);
	        }
	    }
	    else
	    {
	        dprintf(STDERR,"[SORRY]: unable to publish the message\nReason : %s\n",strerror(errno));
	    }
	  }
	  else
	  {
	    dprintf(STDERR,"[SORRY]: unable to send registration\nReason: %s\n",strerror(errno));
	  }

  return STATUS_SUCCESS;
}


/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: RegisterDevice
    Purpose				: This function is used to send Measurement events to the SiteWhere.
    Parameters Passed	: 1) MQTTClient*	-A pointer to an MQTTClient handle. The handle is populated with a
    										 valid client reference following a successful return from this function. 
    					  2) char*			-Device_Name that you want to register with SiteWhere hardware.
    					  3) char*			-a short data_sentence for identifying the record
    					  4) float			-value that has to pass for measurement event.
    Return Parameters	: short				-describe the flow of execution.

    @author Yuvraj Takey
    @version 1.2 19/4/18
---------------------------------------------------------------------------------------------------------------*/
short SendMeasurement(MQTTClient client, char* hardWareID, char* data, float value)
{
  short iRet_Val = DEFAULT_INITIALIZE, exec_STATUS = STATUS_FAILURE;
  uint8_t cData_Buffer[MAXBUFF];
  MQTTClient_deliveryToken token;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;

  if ((iRet_Val = sw_measurement(hardWareID, data, value, DEFALUT_EVENT_TIME, cData_Buffer, sizeof(cData_Buffer), ORIGINATOR, TRUE)) != FALSE)
        {
          pubmsg.payload = cData_Buffer;              // A pointer to the payload of the MQTT message
          pubmsg.payloadlen = sizeof(cData_Buffer);   // Length of the Message 
          pubmsg.qos = QOS;                     	  // the message will be delivered more than once in some circumstances
          pubmsg.retained = FALSE ;

          if((iRet_Val = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) == MQTTCLIENT_SUCCESS)
          {
             if((iRet_Val = MQTTClient_waitForCompletion(client, token, TIMEOUT_MS)) == MQTTCLIENT_SUCCESS)   // wait till publishing not complete
             {
                 printf("[DONE]: Measurement message is delivered with token : %d\n",token);
                 exec_STATUS = STATUS_SUCCESS;
             }
             else
             {
             	dprintf(STDERR,"[SORRY]: unable to publish the message\nReason : %s\n",strerror(errno));
             	exec_STATUS = ERR_PUBLISH;
             }
          }
          else
          {
            dprintf(STDERR,"[SORRY]: unable to publish the message\nReason : %s\n",strerror(errno));
            exec_STATUS = ERR_PUBLISH;
          }
       }
       else
       {
          dprintf(STDERR,"[SORRY]: unable to send Measurement\nReason: %s\n",strerror(errno));
          exec_STATUS = STATUS_FAILURE;
       }
  return exec_STATUS;
}

/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: SendLocation
    Purpose				: This function is used to send Location events to the SiteWhere.
    Parameters Passed	: 1) MQTTClient*	-A pointer to an MQTTClient handle. The handle is populated with a
    										 valid client reference following a successful return from this function. 
    					  2) char*			-Device_Name that you want to register with SiteWhere hardware.
    					  3) float			-latitude value that has to pass for location event.
    					  4) float			-longitude value that has to pass for location event.
    					  5) float			-elevation value that has to pass for location event.
    Return Parameters	: short				-describe the flow of execution.

    @author Yuvraj Takey
    @version 1.2 19/4/18
---------------------------------------------------------------------------------------------------------------*/
short SendLocation(MQTTClient client, char* hardWareID, float latitude, float longitude, float elevation)
{
  short iRet_Val = DEFAULT_INITIALIZE, exec_STATUS = STATUS_FAILURE;
  uint8_t cData_Buffer[MAXBUFF];
  MQTTClient_deliveryToken token;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  
  if ((iRet_Val = sw_location(hardWareID, latitude, longitude, elevation, DEFALUT_EVENT_TIME, cData_Buffer, sizeof(cData_Buffer), ORIGINATOR, TRUE)) != FALSE)
        {
         pubmsg.payload = cData_Buffer;               // A pointer to the payload of the MQTT message
          pubmsg.payloadlen = sizeof(cData_Buffer);   // Length of the Message
          pubmsg.qos = QOS;                     	  // the message will be delivered more than once in some circumstances
          pubmsg.retained = FALSE ;                   // the message it is associated with is being published or received.

          //This function attempts to publish a message to a given topic(MQTT)
          if((iRet_Val = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) == MQTTCLIENT_SUCCESS)
          {
          	//This function is called by the client application to synchronize execution of the 
          	// main thread with completed publication of a message
             if((iRet_Val = MQTTClient_waitForCompletion(client, token, TIMEOUT_MS)) == MQTTCLIENT_SUCCESS)   // wait till publishing not complete
             {
                printf("[DONE]: Location message is delivered with token : %d\n",token);
             	exec_STATUS = STATUS_SUCCESS;
             }
             else
             {
             	dprintf(STDERR,"[SORRY]: unable to publish the message\nReason : %s\n",strerror(errno));
             	exec_STATUS = ERR_PUBLISH;
             }
          }
          else
          {
            dprintf(STDERR,"[SORRY]: unable to publish the message\nReason : %s\n",strerror(errno));
            exec_STATUS = ERR_PUBLISH;
          }
       }
       else
       {
          dprintf(STDERR,"[SORRY]: unable to send Location\nReason: %s\n",strerror(errno));
          exec_STATUS = STATUS_FAILURE;
       }
  return exec_STATUS;
}

/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: SendAlert
    Purpose				: This function is used to send Alert events to the SiteWhere.
    Parameters Passed	: 1) MQTTClient*	-A pointer to an MQTTClient handle. The handle is populated with a
    										 valid client reference following a successful return from this function. 
    					  2) char*			-Device_Name that you want to register with SiteWhere hardware.
    					  3) char*			-Type of alert that has to pass for SiteWhere event.
    					  4) char*			-message for alert that has to pass for SiteWhere event.
    Return Parameters	: short				-describe the flow of execution.

    @author Yuvraj Takey
    @version 1.1 19/4/18
---------------------------------------------------------------------------------------------------------------*/
short SendAlert(MQTTClient client,char* hardWareID, char* alertType, char* alertMessage)
{
  short iRet_Val = DEFAULT_INITIALIZE, exec_STATUS = STATUS_FAILURE;
  uint8_t cData_Buffer[MAXBUFF];
  MQTTClient_deliveryToken token;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;

  if ((iRet_Val = sw_alert(hardWareID, alertType, alertMessage, DEFALUT_EVENT_TIME, cData_Buffer, sizeof(cData_Buffer), ORIGINATOR, TRUE)) != FALSE)
        {
          pubmsg.payload = cData_Buffer;              // A pointer to the payload of the MQTT message
          pubmsg.payloadlen = sizeof(cData_Buffer);   // Length of the Message 
          pubmsg.qos = QOS;                     	  // the message will be delivered more than once in some circumstances
          pubmsg.retained = FALSE;

          if((iRet_Val = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) == MQTTCLIENT_SUCCESS)
          {
             if((iRet_Val = MQTTClient_waitForCompletion(client, token, TIMEOUT_MS)) == MQTTCLIENT_SUCCESS)   // wait till publishing not complete.
             {
                printf("[DONE]: Alert message is delivered with token : %d\n",token);
            	exec_STATUS = STATUS_SUCCESS;
             }
             else
             {
             	dprintf(STDERR,"[SORRY]: unable to publish the message\nReason : %s\n",strerror(errno));
             	exec_STATUS = ERR_PUBLISH;
             }
          }
          else
          {
            dprintf(STDERR,"[SORRY]: unable to publish the message\nReason : %s\n",strerror(errno));
            exec_STATUS = ERR_PUBLISH;
          }
       }
       else
       {
          dprintf(STDERR,"[SORRY]: unable to send Alert\nReason: %s\n",strerror(errno));
          exec_STATUS = STATUS_FAILURE;
       }
  return exec_STATUS;
}

/*-------------------------------------------------------------------------------------------------------------
    Copyright : AxoNet Emsys Pvt. Ltd.

    function			: DisconnectDevice
    Purpose				: This function is used destroy/Disconnect the connection.
    Parameters Passed	: 1) MQTTClient*	-A pointer to an MQTTClient handle. The handle is populated with a
    										 valid client reference following a successful return from this function. 
    					  2) short			-flag that knowing the connection status between Client and Server
    Return Parameters	: short				-none.

    @author Yuvraj Takey
    @version 1.1 18/4/18
---------------------------------------------------------------------------------------------------------------*/
void DisconnectDevice(MQTTClient client)
{ 
	if(iGet_Connect)
	{
      MQTTClient_disconnect(client, TIMEOUT_MS);
      MQTTClient_destroy(&client);
      printf("Client disconnected/destroyed successfully\n");
      iGet_Connect--;			// decrease connection count
  	}
}
