#include "AnyID_CS1110_Rfid.h"
RFID_INFO g_sRfidInfo = {0};

void Rfid_Init()
{
    Rfid_InitInterface(RFID_BAUDRARE);
    Rfid_ConfigInt();
    Rfid_EnableInt(ENABLE,DISABLE);
}

void Rfid_ReceiveFrame(u8 byte, UART_RCVFRAME *pRcvFrame)
{
    switch(pRcvFrame->state)
    {
        case UART_STAT_IDLE:
            if(byte == UART_FRAME_FLAG_HEAD1)
            {
                pRcvFrame->state = UART_STAT_HEAD1;
                pRcvFrame->buffer[0] = UART_FRAME_FLAG_HEAD1;
            }
            break;
        case UART_STAT_HEAD1:
            if(byte == UART_FRAME_FLAG_HEAD2)
            {
                pRcvFrame->state = UART_STAT_HEAD2;
                pRcvFrame->buffer[1] = UART_FRAME_FLAG_HEAD2;
            }
            else if(byte == UART_FRAME_FLAG_HEAD1)
            {
                pRcvFrame->state = UART_STAT_HEAD1;
            }
            else
            {
                pRcvFrame->state = UART_STAT_IDLE;
            }
            break;
        case UART_STAT_HEAD2:
            if(UART_FRAME_DATA_MIN_LEN <= byte && byte <= UART_FRAME_DATA_MAX_LEN)
            {
                pRcvFrame->state = UART_STAT_DATA;
                pRcvFrame->length = (byte & 0xFF) + 3;
                pRcvFrame->buffer[2] = byte;
                pRcvFrame->index = 3;
            }
            else if(byte == 0)
            {
                pRcvFrame->state = UART_STAT_DATA;
                pRcvFrame->length = UART_FRAME_DATA_MIN_LEN + 3;
                pRcvFrame->buffer[2] = byte;
                pRcvFrame->index = 3;
            }
            else
            {
                pRcvFrame->state = UART_STAT_IDLE;
            }
            break;
        case UART_STAT_DATA:
            if(pRcvFrame->buffer[2] == 0)
            {
                if(pRcvFrame->index == 10)
                {
                    pRcvFrame->length += byte;
                }
                else if(pRcvFrame->index == 11)
                {
                    pRcvFrame->length += (byte << 8);
                    pRcvFrame->length += 2;
                    pRcvFrame->length += 1;
                    if(pRcvFrame->length > UART_BUFFER_MAX_LEN)
                    {
                        pRcvFrame->state = UART_STAT_IDLE;
                    }
                }
            }
            pRcvFrame->buffer[pRcvFrame->index++] = byte;
            if(pRcvFrame->index  >= pRcvFrame->length)
            {   
                
                pRcvFrame->state = UART_STAT_END;
            }
            break;
    }

    pRcvFrame->idleTime = 0;
}


u8 Rfid_Format_GetUid(u8 cmd,  RFID_INFO *pOpResult)
{
    u16 pos = 0;
    u16 crc = 0;
    
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pOpResult->buffer[pos++] = 0x00;   // length
    pOpResult->buffer[pos++] = (0000 >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (0000 >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (0001 >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (0001 >> 8) & 0xFF;
    pOpResult->buffer[pos++] = cmd;
    pOpResult->buffer[pos++] = UART_FRAME_PARAM_RFU;
 
    pOpResult->buffer[pos++] = (0x00) & 0xFF;
    pOpResult->buffer[pos++] = (0x01) & 0xFF;
    pOpResult->buffer[pos++] = (0x00) & 0xFF;
    pOpResult->buffer[pos++] = (0x00) & 0xFF;
    pOpResult->buffer[pos++] = (0x00) & 0xFF;
    pOpResult->buffer[pos++] = (0x00) & 0xFF;
    pOpResult->buffer[pos++] = (0x00) & 0xFF;
    
    
    pOpResult->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pOpResult->buffer[pos++] = crc & 0xFF;
    pOpResult->buffer[pos++] = (crc >> 8) & 0xFF;

    pOpResult->len = pos;
    return pos;


}


void Rfid_TransmitCmd(u32 tick)
{
    g_sRfidInfo.tick = tick;
    g_sRfidInfo.len = Rfid_Format_GetUid(RFID_GET_UID, &g_sRfidInfo);
    Rfid_WriteBuffer(g_sRfidInfo.buffer, g_sRfidInfo.len);
}

