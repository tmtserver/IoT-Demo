

/* this is gateway main functionality code
* It is a simple tcp client & a modbus master
*/

// header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <time.h>
#include <errno.h>
#include <modbus.h>
#include "serialize.h"            // it is a user defined header file


struct  readData     s1;
struct  writeData    s2;
struct  pinStatus    s3;
struct  pinResponse  s4;
struct  heartBeatResponse          s5;

// prints error messages to screen
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// MODBUS functions
//function 0x02
/* read discrete input (0x02 function code) */
// this function reads digital input values (ON / OFF)
void read_Discrete_Input(modbus_t *ctx, uint8_t *bits)
{
	int ret,count;
  	ret = modbus_read_input_bits(ctx,STARTING_ADDRESS,LENGTH_DIG, bits);
  	printf("Inside Digital Read\n");
  	if (ret < 0) 
  	   {
	    	fprintf(stderr, "%s\n", modbus_strerror(errno));
	   }   
	else
	   {
	    	printf("BITS DISCRETE:\n");
	        for (count = 0; count < ret; count++)
	        {
		        printf("[%d]=%d\n", count+STARTING_ADDRESS, bits[count]);
	    	}
	    }
}

//function 0x04
/* read input registers (0x04 function code) */
// this function reads analog values (e.g 0-5 v)
void read_Input_Registers(modbus_t *ctx, uint16_t *regs)
{
	int ret,count;
	ret = modbus_read_input_registers(ctx,STARTING_ADDRESS,LENGTH_ANA, regs);
  	if (ret < 0) 
  	{
  	    fprintf(stderr, "%s\n", modbus_strerror(errno));
	}
	else
	{
	    printf("INPUT REGISTERS:\n");
	for (count = 0; count < ret; count++) 
	{
	    printf("[%d]=%d\n", count+STARTING_ADDRESS,regs[count]);
	}
	}
}
//function 0x05
/* write single coil (0x05 function code) */
// this function turns on single digital output 
int write_Single_Coil(modbus_t *ctx, int address, int status)
{
	int ret;
  
  	ret = modbus_write_bit(ctx, address, status);
	if (ret < 0)
	{
	    fprintf(stderr, "%s\n", modbus_strerror(errno));
	}
	return ret;
}
//function 0x0F
/* write multiple coils (0x0F function code) */
// this function turns on multiple digital outputs
int write_Multiple_Coils(modbus_t *ctx, uint8_t *bits)
{
  	int ret;
  	ret = modbus_write_bits(ctx, STARTING_ADDRESS,LENGTH_DIG, bits);
	if (ret < 0) 
	{
	    fprintf(stderr, "%s\n", modbus_strerror(errno));
	}
	return ret;
}
// this function reads data from sensors using modbus functions & sends data to backend server over TCP/IP

int readSensors(modbus_t *ctx, unsigned char *buffer, struct  pinStatus *psptr,struct  tm *timeinfo, int sockfd)
{
    unsigned char *ptr;
    int n;
    
    // generating response packet
	psptr->startByte=0x00FF;  
    psptr->macId=0x03E8;
    psptr->responseId=0x0A;
    psptr->sequenceId[0]=(timeinfo->tm_year + 1900) >> 8;
    psptr->sequenceId[1]=timeinfo->tm_year + 1900 ;
    psptr->sequenceId[2]=timeinfo->tm_mon + 1;
    psptr->sequenceId[3]=timeinfo->tm_mday;
    psptr->sequenceId[4]=timeinfo->tm_hour;
    psptr->sequenceId[5]=timeinfo->tm_min;
    psptr->sequenceId[6]=timeinfo->tm_sec;
    psptr->payloadLength=(16*sizeof(psptr->analogIn[0]))+(8*sizeof(psptr->digitalIn[0]));
    read_Input_Registers(ctx,psptr->analogIn);
    read_Discrete_Input(ctx,psptr->digitalIn);
	psptr->endByte=0xBE;
	
	printf("analog1 %d\n",psptr->analogIn[0]);
	printf("analog2 %d\n",psptr->analogIn[1]);
	
	// this function call serializes data 1 Byte at a times to be sent into a buffer
	ptr =serialize_pinStatus(buffer,psptr);
    printf("size of pin status is %d\n",sizeof(struct pinStatus));
    // sends data over tcp/ip
	n = send(sockfd,buffer,54,0);
	if(n<0)
	{
		perror("error on writing\n");
		exit(1);
    }

    return 0;
}     

// this function writes data to actuators using modbus functions & gives the result back to server 
int writeActuators(modbus_t *ctx, unsigned char *buffer, struct  writeData *wdptr,struct  pinResponse *prptr,struct  tm * timeinfo,int sockfd)
{
    unsigned char *ptr;
    int n,localvar;
    int flag=0,returnval;
 
	// generating the response packet      
    prptr->startByte=0x00FF;
    prptr->macId=0x03E8;
    prptr->responseId=0x0B;
    prptr->sequenceId[0]=(timeinfo->tm_year + 1900) >> 8;
    prptr->sequenceId[1]=timeinfo->tm_year + 1900 ;
    prptr->sequenceId[2]=timeinfo->tm_mon + 1;
    prptr->sequenceId[3]=timeinfo->tm_mday;
    prptr->sequenceId[4]=timeinfo->tm_hour;
    prptr->sequenceId[5]=timeinfo->tm_min;
    prptr->sequenceId[6]=timeinfo->tm_sec;

    prptr->payloadLength=sizeof(prptr->channelNo) + sizeof(prptr->status);
    if((wdptr->content) > 0)
        {
            flag = 1;
    //        printf("code is in flag==1\n");
        }
    else
        {
            flag = 0;
    //         printf("code is in flag==0\n");
         }
    returnval=write_Single_Coil(ctx,(wdptr->channelNo-1),wdptr->content);
    prptr->channelNo = wdptr->channelNo;
    if(flag == 0)
        prptr->status=(returnval-1);
    else
        prptr->status=returnval;
    prptr->endByte=0xBE;
    
    //printf("flag is :%d\n",flag);
    if(returnval > 0)
    {
  	// this function call serializes data to be sent into a buffer
        ptr =serialize_pinResponse(buffer,prptr);
    // sends data over tcp/ip
    printf("size of pin response is %d\n",sizeof(struct pinResponse));    
	    n = send(sockfd,buffer,16,0);
	    if(n<0)
        	{
		        perror("error on writing\n");
		        exit(1);
            }
    }
    return 0;
}


//  2.heart beat command (Total Size 15 Bytes)
void getHeartBeat (modbus_t *ctx, unsigned char *buffer, struct  heartBeatResponse *hbptr,struct  tm *timeinfo, int sockfd)
{
    unsigned char *ptr;
    int n;
    hbptr->startByte=0x00FF;
    hbptr->macId=0x03E8;
    hbptr->responseId=0x0C;
    hbptr->sequenceId[0]=(timeinfo->tm_year + 1900) >> 8;
    hbptr->sequenceId[1]=timeinfo->tm_year + 1900 ;
    hbptr->sequenceId[2]=timeinfo->tm_mon + 1;
    hbptr->sequenceId[3]=timeinfo->tm_mday;
    hbptr->sequenceId[4]=timeinfo->tm_hour;
    hbptr->sequenceId[5]=timeinfo->tm_min;
    hbptr->sequenceId[6]=timeinfo->tm_sec;
    hbptr->stopByte=0xBE;
	ptr =serialize_heartBeatResponse(buffer,hbptr);
    printf("sie of heart beat is %d\n",sizeof(struct heartBeatResponse));
	n = send(sockfd,buffer,13,0);
	if(n<0)
	{
		perror("error on writing\n");
		exit(1);
   	}
       
}
  

int main(int argc, char *argv[])
{
    
    struct  sockaddr_in serv_addr;
    struct  hostent *server;
    struct  tm * timeinfo;
    uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;
    struct  readData     *rdptr;
    struct  writeData    *wdptr;
    struct  pinStatus    *psptr;
    struct  pinResponse  *prptr;
    struct  heartBeatResponse         *hbptr;
    time_t  rawtime;
    modbus_t   *ctx;
    pid_t id;

    unsigned char buffer[1024], *ptr;
    unsigned char compare[5];    
    int sockfd, portno;
    int n,flag=0;

    uint8_t     bits[MODBUS_MAX_READ_BITS] = {0};
   	uint16_t    regs[MODBUS_MAX_READ_REGISTERS] = {0};
   	
   	int i=0;
    
   // checks if enough arguments are passed i.e ip address & port number
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    
    // assigning pointers to corresponding structures
    rdptr=&s1;
    wdptr=&s2;
    psptr=&s3;
    prptr=&s4;
    hbptr=&s5;
     
    
    // timing related
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

//  getting port no    
    portno = atoi(argv[2]);
    
//  creating a socket    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
        
//  getting the host/server
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
       exit(0);
    }
    
//  initializing TCP/IP structures
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    
//  connecting to server    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");  
         
    printf("server connecction established\n");
    
    
//  creating a news modbus context    
//	ctx = modbus_new_tcp(MODBUS_SERVER_IP, MODBUS_SERVER_PORT);
	ctx = modbus_new_rtu(UART_PORT, BAUD_RATE, PARITY, BYTESIZE, STOPBITS);
	if (ctx == NULL) 
	{
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	  	//setting the slave number	
  	modbus_set_slave(ctx, SERVER_ID);
  	 
	// setting debug mode ON
	modbus_set_debug(ctx, TRUE);
	
	//setting error recover ON
	modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);

/*	
  	//setting the slave number	
  	modbus_set_slave(ctx, SERVER_ID);
*/  	

    modbus_rtu_set_serial_mode(ctx,MODBUS_RTU_RS485);
    //modbus_rtu_set_rts(ctx,MODBUS_RTU_RTS_UP);
    modbus_get_response_timeout(ctx,&old_response_to_sec,&old_response_to_usec);
    modbus_set_response_timeout(ctx,0,750000);

    //connecting to modbus slave
        if (modbus_connect(ctx) == -1) 
	{
		fprintf(stderr, "Connexion failed: %s\n",modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	
		
	id=fork();
	if(id == 0)
	{   	
	while(1)
	  {
	     //sending the heartbeat
	     //readSensors(ctx,buffer,psptr,timeinfo,sockfd); 
        printf("Sending Heartbeat\n");
	     getHeartBeat(ctx,buffer,hbptr,timeinfo,sockfd); 
	     sleep(10);
	  }
	  	
	}
	else
	{
	//        printf("parent process running\n");
	// to run code all time
    while(1)
    {
        
        printf("entering loop for %d time\n",i+1);
        // clearing buffer VIP	   
	    bzero(buffer,1024);
	    
	    //receiving data from TCP/IP socket
    	n = recv(sockfd, buffer, sizeof(buffer), 0);
	    printf("received socket data for %d time\n",i+1);	
	    printf("buffer [4] is %d\n",buffer[4]);       ;
        if(n<0)
        {
	    	perror("error on reading\n");
	    	exit(1);
    	}
    
	    strncpy(compare,buffer,5);
	    //printf("\n");
	    /*
	    for(i=0;i<25;i++)
	    {
	        printf("%02x\n", buffer[i]);
	    }
	    */
	    //printf("\n");
	    //printf("%c",compare[4]);
	    
//we need to identifiy commands using their id.Buffer [4] contains command id.	    
	    switch(buffer[4])

	    {   
	        case 0x0A:ptr =deserialize_readData(buffer,rdptr);                // take data from buffer & arrange in proper format as per defined structures
	                readSensors(ctx,buffer,psptr,timeinfo,sockfd);            // this function reads data from sensors & sends data to server through tcp/ip
	                break;
	        case 0x0B:ptr =deserialize_writeData(buffer,wdptr);               // take data from buffer & arrange in proper format as per defined structures
	                writeActuators(ctx,buffer,wdptr,prptr,timeinfo,sockfd);   // this function reads data from sensors & sends data to server through tcp/ip
	                break;
	        default :printf("Enter a valid choice\n");
	                flag=1;
		        break;
	    }
	    i++;
	    if(flag ==1)
	        break;
	}
	
	/*
	while(1)
	{
	printf("entering loop for %d time\n",i+1);
	readSensors(ctx,buffer,psptr,timeinfo,sockfd);
	sleep(3);
	writeActuators(ctx,buffer,wdptr,prptr,timeinfo,sockfd);
	i++;
	sleep(5);
	}
	*/
	
	
	}
	
	/* Close the connection */
	modbus_close(ctx);
 	modbus_free(ctx);
    close(sockfd);
   
    return 0;
}
