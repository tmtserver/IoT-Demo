
// library header files

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <modbus.h>
#include <mcp23017.h>

// user defined header files
#include "expanderPi.h"

//#define	B_SIZE	4096
#define	SPI_A2D		      0
#define	SPI_D2A		      1

//functionality for RTU

#define MODBUS_DEVICE_ID              1
#define MODBUS_DEBUG                  ON
#define SERVER_ID                     1
#define UART_PORT                     "/dev/ttyAMA0"
#define BAUD_RATE                    9600
#define PARITY                        'N'
#define BYTESIZE                      8
#define STOPBITS                      1 

// functionality for TCP
#define MODBUS_SERVER_IP            "192.168.10.113"
#define MODBUS_SERVER_PORT            1502

void update_gpio_write(modbus_mapping_t *mb_mapping)
{
	int i;
	printf("inside digital write function\n");
    // loop over pins
    for ( i=0; i < 4; i++)
    {
    //	printf("inside digital write function\n");
        digitalWrite(100+i, (mb_mapping->tab_bits[i] != 0) );
       
    }
    printf("\n");
    printf("data written\n");
}

void update_gpio_read(modbus_mapping_t * mb_mapping)
{

	int i;
    printf("inside digital read function\n");
    // loop over pins
    for(i=4; i < 8; i++)
    {
    	mb_mapping->tab_input_bits[i-4] = digitalRead(100+i); 
    //	printf("digital inputs :%d\n",	mb_mapping->tab_input_bits[i-4]  );
    }
   	printf("\n");
/*
	    for(i=4; i < 8; i++)
    {
	
	    printf("digital inputs :%d\t",	mb_mapping->tab_input_bits[i-4]  );
	}
*/
}


int main(int argc, char* argv[])
{
	  
    uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;
    int socket;
	int i,header_length;
	int rc;
	uint8_t *query;
	float inputValue;
	float inputVoltage;
	modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    
	  
	
	// GPIO initialization
	if (wiringPiSetup () == -1)
	     return 1;
	
	//I2C initialization
    mcp23017Setup(100,0x20);
    
	// set pin modes :write:
	for(i=0; i < 4; i++)
	   {
	       pinMode(100+i, OUTPUT);
	   //    digitalWrite(i, HIGH);
	   }
    // set pin modes:read:
    for(i=4; i < 8; i++)
	   {
	       pinMode(100+i, INPUT);
	       
	   }
  
    // wiringpi initialization process
    //wiringPiSetupSys ();
    

    
    // ADC-DAC initialization process
    expanderpiAnalogSetup(200);
    
    // modbus RTU initialization process
  	ctx = modbus_new_rtu(UART_PORT, BAUD_RATE, PARITY, BYTESIZE, STOPBITS);

/* modbus tcp related
    ctx=modbus_new_tcp(MODBUS_SERVER_IP,MODBUS_SERVER_PORT);
*/
	if (ctx == NULL) 
	{
	    fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	modbus_set_slave(ctx, SERVER_ID);
	modbus_set_debug(ctx, TRUE);
	modbus_rtu_set_serial_mode(ctx,MODBUS_RTU_RS485);
	// creating modbus registers
	mb_mapping = modbus_mapping_new(20,20,20,20);
	if (mb_mapping == NULL)
	{
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
	    modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	//modbus_set_slave(ctx, SERVER_ID);
	query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    
    header_length=modbus_get_header_length(ctx);
	modbus_get_response_timeout(ctx,&old_response_to_sec,&old_response_to_usec);
		
/* Define a new and too short timeout! */
//		response_timeout.tv_sec = 3;
//		response_timeout.tv_usec = 0;
    modbus_set_response_timeout(ctx,0,1000000);

/* tcp related    
	socket=modbus_tcp_listen(ctx,1);
	modbus_tcp_accept(ctx,&socket);
*/
	//modbus_rtu_set_rts(ctx,MODBUS_RTU_RTS_DOWN);
	//RTU stuff
    // initializaing master-slave connection
	if (modbus_connect(ctx) == -1)
	{
		fprintf(stderr, "Connexion failed: %s\n",modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	
	
	for (;;)
	{
	    rc = modbus_receive(ctx, query);
		if (rc == -1)
		    {
				break;
			}
	printf("Header is :%x \n",query[header_length]);
	
	if(query[header_length] == 0x04)
	{
	//read analog values
	// currently read temperature sensor
	mb_mapping->tab_input_registers[0] = analogRead (200) ;
	
	// currently read temperature sensor
	mb_mapping->tab_input_registers[1] = analogRead (201) ;
	
	inputValue=analogRead (200);
	printf("input voltage channel 1 :%0.2f\n" ,(inputValue * 4.096)/ 4096 );
	printf("room temperature :%f\n",inputValue/10);
	
	inputValue=analogRead (201);
	inputVoltage=(inputValue * 4.096)/ 4096 ;
	printf("input voltage channel 2: %0.2f\n" ,inputVoltage);
	printf("light intensity:%f\n",((2500/(inputVoltage*2))-500)/10);	
		
	//read digital pins
	update_gpio_read(mb_mapping);	
		/* rc is the query size */
	modbus_reply(ctx, query, rc, mb_mapping);
	if(rc== -1)
	    {
			break;
		}
	
	}
	

	if(query[header_length] == 0x05)
	{
	    	/* rc is the query size */
	modbus_reply(ctx, query, rc, mb_mapping);
	if(rc== -1)
	    {
			break;
		}
	    update_gpio_write(mb_mapping);
	  
	    for(i=0;i<8;i++)
	    {
		    printf("%d \t",mb_mapping->tab_bits[i]);
	    }
	    printf("\n");
	}
	

	
				
	}
	printf("Quit the loop: %s\n", modbus_strerror(errno));
	modbus_mapping_free(mb_mapping);
	free(query);
//	close(socket);
	modbus_close(ctx);
	modbus_free(ctx);
	return 0;
}

