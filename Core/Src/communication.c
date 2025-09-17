/*
 * communication.c
 *
 *  Created on: Sep 17, 2025
 *      Author: Gemini
 */

#include "communication.h"
#include <string.h>

// RX state machine
typedef enum {
    STATE_WAIT_FOR_SOF,
    STATE_READ_COMMAND,
    STATE_READ_LENGTH,
    STATE_READ_PAYLOAD,
    STATE_READ_CHECKSUM,
    STATE_READ_EOF
} RxState;

static UART_HandleTypeDef* huart_comm;
static RxState rx_state = STATE_WAIT_FOR_SOF;
static uint8_t rx_command;
static uint8_t rx_payload_len;
static uint8_t rx_payload[255];
static uint8_t rx_payload_index;
static uint8_t rx_checksum;

static void process_message(void);
static uint8_t calculate_checksum(uint8_t cmd, uint8_t len, uint8_t* payload);
static void send_packet(uint8_t cmd, uint8_t len, uint8_t* payload);

void Communication_Init(UART_HandleTypeDef *huart) {
    huart_comm = huart;
}

void Communication_Handle_Rx(uint8_t* buf, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        uint8_t byte = buf[i];
        switch (rx_state) {
            case STATE_WAIT_FOR_SOF:
                if (byte == SOF_BYTE) {
                    rx_state = STATE_READ_COMMAND;
                }
                break;
            case STATE_READ_COMMAND:
                rx_command = byte;
                rx_state = STATE_READ_LENGTH;
                break;
            case STATE_READ_LENGTH:
                rx_payload_len = byte;
                if (rx_payload_len == 0) {
                    rx_state = STATE_READ_CHECKSUM;
                } else if (rx_payload_len < sizeof(rx_payload)) {
                    rx_payload_index = 0;
                    rx_state = STATE_READ_PAYLOAD;
                } else {
                    // Payload too large, reset
                    rx_state = STATE_WAIT_FOR_SOF;
                }
                break;
            case STATE_READ_PAYLOAD:
                rx_payload[rx_payload_index++] = byte;
                if (rx_payload_index >= rx_payload_len) {
                    rx_state = STATE_READ_CHECKSUM;
                }
                break;
            case STATE_READ_CHECKSUM:
                rx_checksum = byte;
                rx_state = STATE_READ_EOF;
                break;
            case STATE_READ_EOF:
                if (byte == EOF_BYTE) {
                    process_message();
                }
                rx_state = STATE_WAIT_FOR_SOF;
                break;
        }
    }
}

static void process_message() {
    uint8_t calculated_checksum = calculate_checksum(rx_command, rx_payload_len, rx_payload);
    if (calculated_checksum != rx_checksum) {
        Communication_Send_Error(ERROR_BAD_CHECKSUM);
        return;
    }

    switch (rx_command) {
        case CMD_SET_MOTORS:
            if (rx_payload_len == sizeof(SetMotorsPayload)) {
                SetMotorsPayload* p = (SetMotorsPayload*)rx_payload;
                App_Set_Motors(p->motor1_speed, p->motor2_speed);
                Communication_Send_Ack(rx_command);
            }
            break;
        case CMD_GET_ENCODERS: {
            int32_t enc1, enc2;
            App_Get_Encoders(&enc1, &enc2);
            Communication_Send_Encoder_Data(enc1, enc2);
            break;
        }
        case CMD_RESET_ENCODERS:
            App_Reset_Encoders();
            Communication_Send_Ack(rx_command);
            break;
        case CMD_PING:
            Communication_Send_Pong();
            break;
        case CMD_MOVE_STEPS:
            if (rx_payload_len == sizeof(MoveStepsPayload)) {
                MoveStepsPayload* p = (MoveStepsPayload*)rx_payload;
                App_Move_Steps(p->motor1_steps, p->motor2_steps);
                Communication_Send_Ack(rx_command);
            }
            break;
        default:
            Communication_Send_Error(ERROR_UNKNOWN_CMD);
            break;
    }
}

static uint8_t calculate_checksum(uint8_t cmd, uint8_t len, uint8_t* payload) {
    uint8_t checksum = cmd ^ len;
    for (uint8_t i = 0; i < len; i++) {
        checksum ^= payload[i];
    }
    return checksum;
}

static void send_packet(uint8_t cmd, uint8_t len, uint8_t* payload) {
    uint8_t packet[260]; // Max payload 255 + 5 bytes overhead
    uint8_t index = 0;
    packet[index++] = SOF_BYTE;
    packet[index++] = cmd;
    packet[index++] = len;
    if (len > 0) {
        memcpy(&packet[index], payload, len);
        index += len;
    }
    packet[index++] = calculate_checksum(cmd, len, payload);
    packet[index++] = EOF_BYTE;
    HAL_UART_Transmit(huart_comm, packet, index, HAL_MAX_DELAY);
}

void Communication_Send_Encoder_Data(int32_t encoder1, int32_t encoder2) {
    EncoderDataPayload payload;
    payload.encoder1_val = encoder1;
    payload.encoder2_val = encoder2;
    send_packet(MSG_ENCODER_DATA, sizeof(payload), (uint8_t*)&payload);
}

void Communication_Send_Ack(uint8_t command_id) {
    send_packet(MSG_ACK, 1, &command_id);
}

void Communication_Send_Pong(void) {
    send_packet(MSG_PONG, 0, NULL);
}

void Communication_Send_Error(uint8_t error_code) {
    send_packet(MSG_ERROR, 1, &error_code);
}
