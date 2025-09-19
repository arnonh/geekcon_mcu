
# Geekcon Robot MCU Controller

This project contains the firmware for the microcontroller (MCU) unit of a robot.

## Overview

The MCU is responsible for the low-level control of the robot's movement. It interfaces directly with the motor drivers and encoders.

## Features

*   **Motor Control:** Controls two DC motors.
*   **Encoder Reading:** Reads feedback from two wheel encoders to provide data for speed and distance calculations.

## Hardware

*   **MCU:** STM32L412xx

## Toolchain

*   **Code Generation:** STM32CubeMX
*   **Build System:** CMake
*   **Compiler:** GCC for ARM

## Communication

*   **USART1:** Used for communication between the MCU and a Raspberry Pi.

## Communication Protocol

A binary protocol is used for communication between the Raspberry Pi and the MCU over USART1.

### Message Structure

| Field          | Length (bytes) | Description                                               |
|----------------|----------------|-----------------------------------------------------------|
| Start of Frame | 1              | `0xAA`                                                    |
| Command ID     | 1              | The command identifier.                                   |
| Payload Length | 1              | Length of the Payload field in bytes.                     |
| Payload        | 0-255          | Data associated with the command.                         |
| Checksum       | 1              | XOR checksum of all bytes from Command ID to Payload.     |
| End of Frame   | 1              | `0x55`                                                    |

### Commands (Raspberry Pi -> MCU)

| Command              | ID     | Payload                                     | Description                                       |
|----------------------|--------|---------------------------------------------|---------------------------------------------------|
| `CMD_SET_MOTORS`     | `0x01` | `[motor1_speed(i16), motor2_speed(i16)]`    | Sets the speed for both motors (-1000 to 1000).   |
| `CMD_GET_ENCODERS`   | `0x02` | None                                        | Requests the current encoder counts.              |
| `CMD_RESET_ENCODERS` | `0x03` | None                                        | Resets the encoder counts to zero.                |
| `CMD_PING`           | `0x04` | None                                        | Checks if the MCU is responsive.                  |
| `CMD_MOVE_STEPS`     | `0x05` | `[motor1_steps(i32), motor2_steps(i32)]`    | Moves each motor a specified number of steps.     |

### Messages (MCU -> Raspberry Pi)

| Message            | ID     | Payload                                       | Description                                       |
|--------------------|--------|-----------------------------------------------|---------------------------------------------------|
| `MSG_ENCODER_DATA` | `0x11` | `[encoder1_val(i32), encoder2_val(i32)]`      | Response to `CMD_GET_ENCODERS`.                   |
| `MSG_ACK`          | `0x12` | `[acked_cmd_id(u8)]`                          | Acknowledges a command was received and processed.|
| `MSG_PONG`         | `0x13` | None                                          | Response to `CMD_PING`.                           |
| `MSG_ERROR`        | `0xEE` | `[error_code(u8)]`                            | Reports an error (e.g., bad checksum).            |

### Message Examples

#### `CMD_SET_MOTORS`

Set motor 1 speed to 500 and motor 2 speed to -500.

*   **Start of Frame:** `0xAA`
*   **Command ID:** `0x01`
*   **Payload Length:** `4`
*   **Payload:** `0x01F4` (500), `0xFE0C` (-500) -> `F4 01 0C FE` (little-endian)
*   **Checksum:** `0x01 ^ 0x04 ^ 0xF4 ^ 0x01 ^ 0x0C ^ 0xFE = 0x02`
*   **End of Frame:** `0x55`

**Message:** `AA 01 04 F4 01 0C FE 02 55`

#### `MSG_ENCODER_DATA`

Encoder 1 value is 10000 and encoder 2 value is 20000.

*   **Start of Frame:** `0xAA`
*   **Message ID:** `0x11`
*   **Payload Length:** `8`
*   **Payload:** `10000`, `20000` -> `10 27 00 00 20 4E 00 00` (little-endian)
*   **Checksum:** `0x11 ^ 0x08 ^ 0x10 ^ 0x27 ^ 0x00 ^ 0x00 ^ 0x20 ^ 0x4E ^ 0x00 ^ 0x00 = 0x40`
*   **End of Frame:** `0x55`

**Message:** `AA 11 08 10 27 00 00 20 4E 00 00 40 55`
