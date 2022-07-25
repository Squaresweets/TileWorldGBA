#include "ws.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//Reference: https://github.com/samjkent/picow-websocket/blob/main/ws.cpp

/*
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-------+-+-------------+-------------------------------+
 *   |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 *   |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 *   |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 *   | |1|2|3|       |K|             |                               |
 *   +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 *   |     Extended payload length continued, if payload len == 127  |
 *   + - - - - - - - - - - - - - - - +-------------------------------+
 *   |                               |Masking-key, if MASK set to 1  |
 *   +-------------------------------+-------------------------------+
 *   | Masking-key (continued)       |          Payload Data         |
 *   +-------------------------------- - - - - - - - - - - - - - - - +
 *   :                     Payload Data continued ...                :
 *   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 *   |                     Payload Data continued ...                |
 *   +---------------------------------------------------------------+
 */

uint64_t WSBuildPacket(char* buffer, uint64_t bufferSize, enum WSOpCode opcode, char* payload, uint64_t payloadLen)
{
    WebsocketPacketHeader_t header;

    uint64_t payloadIndex;

    //Set meta bytes
    header.meta.bits.FIN = 1;
    header.meta.bits.RSV = 0;
    header.meta.bits.OPCODE = opcode;
    header.meta.bits.MASK = 1;

    //Calculate length, if the length is 0-125 then the payload is just in those 7 bits
    //If the length fits into a 16 bit unsigned integer the payload length is set as 126
    //If the length fits into a 64 bit unsigned integer (minus the most significant bit) the payload length is set as 127
    if(payloadLen < 126) header.meta.bits.PAYLOADLEN = payloadLen;
    else if(payloadLen < 0x10000) header.meta.bits.PAYLOADLEN = 126;
    else header.meta.bits.PAYLOADLEN = 127;

    buffer[payloadIndex++] = header.meta.bytes.byte0;
    buffer[payloadIndex++] = header.meta.bytes.byte1;

    if(header.meta.bits.PAYLOADLEN == 126)
    {
        buffer[payloadIndex++] = (payloadLen >> 8) & 0xFF;
        buffer[payloadIndex++] = payloadLen & 0xFF;
    }
    else if(header.meta.bits.PAYLOADLEN == 127)
    {
        memcpy(&buffer[payloadIndex], &payloadLen, 8);
        payloadIndex += 8;
    }

    //Mask payload
    header.mask.maskKey = (uint32_t)rand();

    for(uint64_t i = 0; i < payloadLen; i++)
        payload[i] ^= header.mask.maskBytes[i%4];

    //Add the masking key
    buffer[payloadIndex++] = header.mask.maskBytes[0];
    buffer[payloadIndex++] = header.mask.maskBytes[1];
    buffer[payloadIndex++] = header.mask.maskBytes[2];
    buffer[payloadIndex++] = header.mask.maskBytes[3];
    
    if((payloadLen + payloadIndex) > bufferSize)
        printf("*****************WEBSOCKET BUFFER OVERFLOW*****************\r\n");

    //Copy payload
    for(int i = 0; i < payloadLen; i++)
        buffer[payloadIndex + i] = payload[i];

    return payloadIndex + payloadLen;
}

void WSParsePacket(WebsocketPacketHeader_t *header, char* buffer, uint32_t len)
{
    int payloadIndex;
    header->meta.bytes.byte0 = buffer[payloadIndex++];
    header->meta.bytes.byte1 = buffer[payloadIndex++];

    header->length = header->meta.bits.PAYLOADLEN;

    if(header->meta.bits.PAYLOADLEN == 126)
    {
        printf("This is probably going to be the issue\n");
        header->length = buffer[payloadIndex + 1] << 8 | buffer[payloadIndex];
        printf("header->length: %d", header->length);
        payloadIndex += 2;
    }
    else if(header->meta.bits.PAYLOADLEN == 127)
    {
        memcpy(&header->length, &buffer[payloadIndex], 8);
        payloadIndex += 8;
    }

    if(header->meta.bits.MASK)
    {
        printf("MASKING PACKET FROM SERVER, this should never happen\n"); 
        header->mask.maskBytes[0] = buffer[payloadIndex++];
        header->mask.maskBytes[1] = buffer[payloadIndex++];
        header->mask.maskBytes[2] = buffer[payloadIndex++];
        header->mask.maskBytes[3] = buffer[payloadIndex++];
        printf("%#X\n", header->mask.maskKey);
        
        // Decrypt    
        for(uint64_t i = 0; i < header->length; i++) 
            buffer[payloadIndex++] ^= header->mask.maskBytes[i%4];
    }
    memmove(buffer, &buffer[payloadIndex], header->length); //Move the unencrypted data back to the start
}