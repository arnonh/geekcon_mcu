/*
 * communication.h
 *
 *  Created on: Sep 17, 2025
 *      Author: Gemini
 */

#ifndef INC_COMMUNICATION_H_
#define INC_COMMUNICATION_H_

#include "main.h"
#include <stdint.h>

// Protocol constants
#define SOF_BYTE 0xAA
#define EOF_BYTE 0x55

// Command IDs (RPi -> MCU)
#define CMD_SET_MOTORS      0x01
#define CMD_GET_ENCODERS    0x02
#define CMD_RESET_ENCODERS  0x03
#define CMD_PING            0x04
#define CMD_MOVE_STEPS      0x05
#define CMD_GET_MODE        0x06

// Message IDs (MCU -> RPi)
#define MSG_ENCODER_DATA    0x11
#define MSG_ACK             0x12
#define MSG_PONG            0x13
#define MSG_MODE_DATA       0x14
#define MSG_ERROR           0xEE

// Error codes
#define ERROR_BAD_CHECKSUM  0x01
#define ERROR_UNKNOWN_CMD   0x02

#pragma pack(push, 1)

// Payload Structures
typedef struct {
    int16_t motor1_speed;
    int16_t motor2_speed;
} SetMotorsPayload;

typedef struct {
    int32_t motor1_steps;
    int32_t motor2_steps;
} MoveStepsPayload;

typedef struct {
    int32_t encoder1_val;
    int32_t encoder2_val;
} EncoderDataPayload;

typedef struct {
    uint8_t mode;
} ModeDataPayload;

#pragma pack(pop)

// Public function prototypes
void Communication_Init(UART_HandleTypeDef *huart);
void Communication_Handle_Rx(uint8_t* buf, uint32_t len);
void Communication_Send_Encoder_Data(int32_t encoder1, int32_t encoder2);
void Communication_Send_Ack(uint8_t command_id);
void Communication_Send_Pong(void);
void Communication_Send_Error(uint8_t error_code);
void Communication_Send_Mode(RobotMode mode);

// Callback function prototypes that must be implemented by the application
void App_Set_Motors(int16_t motor1, int16_t motor2);
void App_Get_Encoders(int32_t* encoder1, int32_t* encoder2);
void App_Reset_Encoders(void);
void App_Move_Steps(int32_t motor1_steps, int32_t motor2_steps);
RobotMode App_Get_Mode(void);


#endif /* INC_COMMUNICATION_H_ */
