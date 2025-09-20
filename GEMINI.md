This project contains the firmware for the microcontroller (MCU) unit of a robot.

## Overview

The MCU is responsible for the low-level control of the robot's movement. It interfaces directly with the motor drivers and encoders.

## Features

*   **Motor Control:** Controls two DC motors.
*   **Encoder Reading:** Reads feedback from two wheel encoders to provide data for speed and distance calculations.

## Communication

*   **USART1:** Used for communication between the MCU and a Raspberry Pi.

## Hardware

*   **MCU:** STM32L412xx

## Toolchain

*   **Code Generation:** STM32CubeMX
*   **Build System:** CMake
*   **Compiler:** GCC for ARM

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
| `CMD_GET_MODE`       | `0x06` | None                                        | Requests the current robot mode.                  |

### Messages (MCU -> Raspberry Pi)

| Message            | ID     | Payload                                       | Description                                       |
|--------------------|--------|-----------------------------------------------|---------------------------------------------------|
| `MSG_ENCODER_DATA` | `0x11` | `[encoder1_val(i32), encoder2_val(i32)]`      | Response to `CMD_GET_ENCODERS`.                   |
| `MSG_ACK`          | `0x12` | `[acked_cmd_id(u8)]`                          | Acknowledges a command was received and processed.|
| `MSG_PONG`         | `0x13` | None                                          | Response to `CMD_PING`.                           |
| `MSG_MODE_DATA`    | `0x14` | `[mode(u8)]`                                  | Response to `CMD_GET_MODE`.                       |
| `MSG_ERROR`        | `0xEE` | `[error_code(u8)]`                            | Reports an error (e.g., bad checksum).            |

### Robot Modes

| Mode         | Value | Description                                                                 |
|--------------|-------|-----------------------------------------------------------------------------|
| `MODE_STOP`  | 0     | The robot is stopped.                                                       |
| `MODE_STEP`  | 1     | The robot is moving a specific number of steps.                             |
| `MODE_SPEED` | 2     | The robot's motors are being controlled directly by speed.                  |