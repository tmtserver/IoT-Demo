/*
 * expanderPi.c:
 *	Access routines for the SPI devices on the expanderpi
 *	Copyright (c) 2012 Gordon Henderson
 *
 *	The expanderpi has:
 *
 *		An MCP3002 dual-channel A to D convertor connected
 *		to the SPI bus, selected by chip-select A, and:
 *
 *		An MCP4802 dual-channel D to A convertor connected
 *		to the SPI bus, selected via chip-select B.
 *
 */


#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "expanderPi.h"

// The A-D convertor won't run at more than 1MHz @ 3.3v

#define	SPI_ADC_SPEED	1000000
#define	SPI_DAC_SPEED	1000000
#define	SPI_A2D		      0
#define	SPI_D2A		      1


/*
 * expanderpiAnalogWrite:
 *	Write an 12-bit data value to the MCP4822 Analog to digital
 *	convertor on the expanderpi.
 *********************************************************************************
 */


 void mcp4822AnalogWrite (const int chan, const int value)
{
  uint8_t spiData [2] ;
  uint8_t chanBits, dataBits ;
  

  if (chan == 0)
    chanBits = 0x30 ;
  else
    chanBits = 0xB0 ;

  chanBits |= ((value >> 8) & 0x0F) ;
  dataBits  = (value & 0xFF) ;

  spiData [0] = chanBits ;
  spiData [1] = dataBits ;

  wiringPiSPIDataRW (SPI_D2A, spiData, 2) ;
}


/*
 * expanderpiAnalogRead:
 *	Return the analog value of the given channel (0/1).
 *	The A/D is a 12-bit device
 *********************************************************************************
 */


 int mcp3208AnalogRead (const int chan)
{
  uint8_t spiData [3] ;
  uint8_t chanBits ;
  

  chanBits = 0b00000110 | (chan >>2)  ;

  spiData [0] = chanBits ;	
  spiData [1] = (chan << 6);
  spiData [2] = 0 ;

  wiringPiSPIDataRW (SPI_A2D, spiData, 3) ;

  return (((spiData [1] << 8) | spiData [2]) & 0xFFF );
}

/*
 * expanderpiSPISetup:
 *	Initialise the SPI bus, etc.
 *********************************************************************************
 */

int expanderpiSPISetup (void)
{
  if (wiringPiSPISetup (SPI_A2D, SPI_ADC_SPEED) < 0)
    return -1 ;

  if (wiringPiSPISetup (SPI_D2A, SPI_DAC_SPEED) < 0)
    return -1 ;

  return 0 ;
}


/*
 * New wiringPi node extension methods.
 *********************************************************************************
 */

static int myAnalogRead (struct wiringPiNodeStruct *node, const int chan)
{
  return  mcp3208AnalogRead (chan - node->pinBase) ;
}

static void myAnalogWrite (struct wiringPiNodeStruct *node, const int chan, const int value)
{
   mcp4822AnalogWrite (chan - node->pinBase, value) ;
}


/*
 * expanderpiAnalogSetup:
 *	Create a new wiringPi device node for the analog devices on the
 *	expanderpi. We create one node with 2 pins - each pin being read
 *	and write - although the operations actually go to different
 *	hardware devices.
 *********************************************************************************
 */

int expanderpiAnalogSetup (const int pinBase)
{
  struct wiringPiNodeStruct *node ;
  int    x ;

  if (( x = expanderpiSPISetup ()) != 0)
    return  x;

  node = wiringPiNewNode (pinBase, 8) ;
  node->analogRead  = myAnalogRead ;
  node->analogWrite = myAnalogWrite ;

  return 0 ;
}
