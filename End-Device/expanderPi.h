/*
 * expanderPi.h:
 *
*/

#ifndef expanderPi_H
#define expanderPi_H


extern void mcp4822AnalogWrite (const int chan, const int value);
extern int mcp3208AnalogRead (const int chan);
extern int expanderpiSPISetup (void);
extern int expanderpiAnalogSetup (const int pinBase);

#endif
