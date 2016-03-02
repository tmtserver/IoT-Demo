



#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "serialize.h"

// serialization functions
unsigned char *serialize_char(unsigned char *buffer, char value)
{
    buffer[0] = value;
    return buffer + 1;
}
unsigned char *serialize_8int(unsigned char *buffer, uint8_t value)
{ 
    buffer[0] =value ;
    return buffer + 1;
}
unsigned char *serialize_16int(unsigned char *buffer, uint16_t value)
{
    /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
    buffer[0] = value >> 8;
    buffer[1] = value;
    return buffer + 2;
}

unsigned char *serialize_32int(unsigned char *buffer, uint32_t value)
{
 /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
    buffer[0] = value >> 24;
    buffer[1] = value >> 16;
    buffer[2] = value >> 8;
    buffer[3] = value;
    return buffer + 4;
}

// this function serializes readData structure into buffer
unsigned char *serialize_readData(unsigned char *buffer, struct  readData *value)
{
    int i;
    buffer=serialize_16int(buffer, value->startByte);        //startbyte
    buffer=serialize_16int(buffer, value->macId);            //macid
    buffer=serialize_8int(buffer, value->commandId);        //commandid
    for(i=0;i<=6;i++)
    {
        buffer=serialize_8int(buffer,value->sequenceId[i]);      //sequenceid
    }
    buffer=serialize_8int(buffer,value->payloadLength);      //payloadlength
    buffer=serialize_8int(buffer,value->read);              //read
    buffer=serialize_8int(buffer,value->endByte);                //crc
    return buffer;
}

// this function serializes writeData structure into buffer

unsigned char *serialize_writeData(unsigned char *buffer, struct  writeData *value)
{
    int i;
    buffer=serialize_16int(buffer, value->startByte);        //startbyte
    buffer=serialize_16int(buffer, value->macId);            //macid
    buffer=serialize_8int(buffer, value->commandId);        //commandid
    for(i=0;i<=6;i++)
    {
        buffer=serialize_8int(buffer,value->sequenceId[i]);      //sequenceid
    }
    buffer=serialize_8int(buffer,value->payloadLength);      //payloadlength
    buffer=serialize_8int(buffer,value->channelNo);              //channel number
    buffer=serialize_8int(buffer,value->content);              //value to be written
  
    buffer=serialize_8int(buffer,value->endByte);                //crc
    return buffer;
}

// this function serializes pinStatus structure into buffer
unsigned char *serialize_pinStatus(unsigned char *buffer, struct  pinStatus *value)
{
    int i;
    buffer=serialize_16int(buffer, value->startByte);        //startbyte
    buffer=serialize_16int(buffer, value->macId);            //macid
    buffer=serialize_8int(buffer, value->responseId);        //responseid
    for(i=0;i<=6;i++)
    {
        buffer=serialize_8int(buffer,value->sequenceId[i]);      //sequenceid
    }
    buffer=serialize_8int(buffer,value->payloadLength);      //payloadlength
    for(i=0;i<=15;i++)
    {
        buffer = serialize_16int(buffer, value->analogIn[i]);    //analogin
    }
    for(i=0;i<=7;i++)
    {
        buffer = serialize_8int(buffer, value->digitalIn[i]);    //digitalin
    }
    buffer=serialize_8int(buffer,value->endByte);                //crc
    return buffer;
}

// this function serializes pinResponse structure into buffer
unsigned char *serialize_pinResponse(unsigned char *buffer, struct  pinResponse *value)
{
    int i;
    buffer=serialize_16int(buffer, value->startByte);        //startbyte
    buffer=serialize_16int(buffer, value->macId);            //macid
    buffer=serialize_8int(buffer, value->responseId);        //responseid
    for(i=0;i<=6;i++)
    {
        buffer=serialize_8int(buffer,value->sequenceId[i]);      //sequenceid
    }
    buffer=serialize_8int(buffer,value->payloadLength);      //payloadlength
    buffer = serialize_8int(buffer, value->channelNo);    // channel no
    buffer = serialize_8int(buffer, value->status);    // status
    buffer=serialize_8int(buffer,value->endByte);                //crc
    return buffer;
}

  
// this function serializes heartBeatResponse

unsigned char *serialize_heartBeatResponse(unsigned char *buffer, struct  heartBeatResponse *value)
{
    int i;
    buffer=serialize_16int(buffer, value->startByte);             //startbyte
    buffer=serialize_16int(buffer, value->macId);                 //macid
    buffer=serialize_8int(buffer, value->responseId);             //responseid
    for(i=0;i<=6;i++)
    {
        buffer=serialize_8int(buffer,value->sequenceId[i]);       //sequenceid
    }
    buffer=serialize_8int(buffer,value->stopByte);                //crc
    
    return buffer;
}

//deserialization functions
unsigned char *deserialize_char(unsigned char *buffer, char *value)
{
    *value=buffer[0];
    return buffer + 1;
}
unsigned char * deserialize_8int(unsigned char *buffer,  uint8_t *value)
{
    *value = buffer[0]; 
    return buffer + 1;
}
unsigned char * deserialize_16int(unsigned char *buffer, uint16_t *value)
{
    uint16_t temp;
    temp=((buffer[0] << 8) |  buffer[1]);
//  printf("code is at first breakpt\n");
//  printf("tmp is %d\n",temp);
    *value= temp;
    return buffer + 2;
}
unsigned char * deserialize_32int(unsigned char *buffer, uint32_t *value)
{
    uint32_t temp;
    temp=((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8)| buffer[3]);
//  printf("code is at second breakpt\n");
//  printf("tmp is %d\n",temp);
    *value= temp;
    return buffer + 4;
}
//  this function deserializes readData structure from buffer
unsigned char *deserialize_readData(unsigned char *buffer, struct  readData *value)
{
    int i;
    buffer=deserialize_16int(buffer, &value->startByte);
    buffer=deserialize_16int(buffer, &value->macId);
    buffer=deserialize_8int(buffer, &value->commandId);
    for(i=0;i<=6;i++)
    {
        buffer=deserialize_8int(buffer,&value->sequenceId[i]);
    }
    buffer=deserialize_8int(buffer,&value->payloadLength);
    buffer=deserialize_8int(buffer,&value->read);
    buffer=deserialize_8int(buffer,&value->endByte);
    return buffer;
}

//  this function deserializes writeData structure from buffer
unsigned char *deserialize_writeData(unsigned char *buffer, struct  writeData *value)
{
    int i;
    int localvar;
    buffer=deserialize_16int(buffer, &value->startByte);
    buffer=deserialize_16int(buffer, &value->macId);
    buffer=deserialize_8int(buffer, &value->commandId);
    for(i=0;i<=6;i++)
    {
        buffer=deserialize_8int(buffer,&value->sequenceId[i]);
    }
    buffer=deserialize_8int(buffer,&value->payloadLength);
    buffer=deserialize_8int(buffer,&value->channelNo);
    buffer=deserialize_8int(buffer,&value->content);
    
     
    buffer=deserialize_8int(buffer,&value->endByte);
    return buffer;
}

// this function deserializes pinStatus structure from buffer
unsigned char *deserialize_pinStatus(unsigned char *buffer, struct  pinStatus *value)
{
    int i;
    buffer=deserialize_16int(buffer, &value->startByte);
    buffer=deserialize_16int(buffer, &value->macId);
    buffer=deserialize_8int(buffer, &value->responseId);
    for(i=0;i<=6;i++)
    {
        buffer=deserialize_8int(buffer,&value->sequenceId[i]);
    }
    buffer=deserialize_8int(buffer,&value->payloadLength);
    for(i=0;i<=15;i++)
    {
        buffer = deserialize_16int(buffer, &value->analogIn[i]);
    }
    for(i=0;i<=7;i++)
    {
        buffer = deserialize_8int(buffer, &value->digitalIn[i]);
    }
    buffer=deserialize_8int(buffer,&value->endByte);
    return buffer;
}

// this function deserializes pinResponse structure from buffer
unsigned char *deserialize_pinResponse(unsigned char *buffer, struct  pinResponse *value)
{
    int i;
    buffer=deserialize_16int(buffer, &value->startByte);
    buffer=deserialize_16int(buffer, &value->macId);
    buffer=deserialize_8int(buffer, &value->responseId);
    for(i=0;i<=6;i++)
    {
        buffer=deserialize_8int(buffer,&value->sequenceId[i]);
    }
    buffer=deserialize_8int(buffer,&value->payloadLength);
    buffer = deserialize_8int(buffer, &value->channelNo);
    buffer = deserialize_8int(buffer, &value->status);
    buffer=deserialize_8int(buffer,&value->endByte);
    return buffer;
}

