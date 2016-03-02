
// gateway main header file 
#ifndef serialize_H
#define serialize_H

#include<stdint.h>

#define MODBUS_SERVER_IP            "192.168.10.113"
#define MODBUS_SERVER_PORT          1502
#define MODBUS_DEBUG                ON
#define SERVER_ID                   1
#define UART_PORT                   "/dev/ttyAMA0"
#define BAUD_RATE                   9600
#define PARITY                      'N'
#define BYTESIZE                    8
#define STOPBITS                    1
#define REGISTER_ADDRESS            97
#define NO_OF_REGISTERS             1
#define STARTING_ADDRESS 0
#define LENGTH 7
#define LENGTH_ANA 16
#define LENGTH_DIG 8


// command type structures
struct readData{                   //total :15B
    uint16_t   startByte;      //2B
    uint16_t   macId;          //2B
    uint8_t    commandId;      //1B   
    uint8_t    sequenceId[7];  //1B*7
    uint8_t    payloadLength;  //1B
    uint8_t    read;           //1B
    uint8_t    endByte;            //1B
    };
        
struct writeData{                  //total :23B
    uint16_t   startByte ;     //2B
    uint16_t   macId;          //2B
    uint8_t    commandId;      //1B   
    uint8_t    sequenceId[7];  //1B*7
    uint8_t    payloadLength;  //1B
    uint8_t    channelNo;      //1B
    uint8_t    content;        //1B
    uint8_t    endByte;            //1B
    };

// response type structures
struct pinStatus{                  //total :54B
    uint16_t   startByte ;     //2B
    uint16_t   macId;          //2B
    uint8_t    responseId;     //1B   
    uint8_t    sequenceId[7];  //1B*7
    uint8_t    payloadLength;  //1B
    uint16_t   analogIn[16];   //2B*16 
    uint8_t    digitalIn[8];   //1B*8
    uint8_t    endByte;            //1B
    };
        
struct pinResponse{                //total :16B
    uint16_t   startByte ;     //2B
    uint16_t   macId;          //2B
    uint8_t    responseId;     //1B   
    uint8_t    sequenceId[7];  //1B*7
    uint8_t    payloadLength;  //1B
    uint8_t    channelNo;      //1B
    uint8_t    status;        //1B
    uint8_t    endByte;            //1B
    };

struct heartBeatResponse{            //total :13Bytes
    uint16_t   startByte;            //2B
    uint16_t   macId;                //2B
    uint8_t    responseId;           //1B   
    uint8_t    sequenceId[7];        //1B*7
    uint8_t    stopByte;             //1B
    };

//serialization functions
unsigned char *serialize_char(unsigned char *buffer, char value);
unsigned char *serialize_8int(unsigned char *buffer, uint8_t value);
unsigned char *serialize_16int(unsigned char *buffer, uint16_t value);
unsigned char *serialize_32int(unsigned char *buffer, uint32_t value);
unsigned char *serialize_readData(unsigned char *buffer, struct  readData *value);
unsigned char *serialize_writeData(unsigned char *buffer, struct  writeData *value);
unsigned char *serialize_pinStatus(unsigned char *buffer, struct  pinStatus *value);
unsigned char *serialize_pinResponse(unsigned char *buffer, struct  pinResponse *value);
unsigned char *serialize_heartBeatResponse(unsigned char *buffer, struct  heartBeatResponse *value);

//deserialization functions

unsigned char *deserialize_char(unsigned char *buffer, char *value);
unsigned char *deserialize_8int(unsigned char *buffer,  uint8_t *value);
unsigned char *deserialize_16int(unsigned char *buffer, uint16_t *value);
unsigned char *deserialize_32int(unsigned char *buffer, uint32_t *value);
unsigned char *deserialize_readData(unsigned char *buffer, struct  readData *value);
unsigned char *deserialize_writeData(unsigned char *buffer, struct  writeData *value);
unsigned char *deserialize_pinStatus(unsigned char *buffer, struct  pinStatus *value);
unsigned char *deserialize_pinResponse(unsigned char *buffer, struct  pinResponse *value);



#endif











