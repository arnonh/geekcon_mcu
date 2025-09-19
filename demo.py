import serial
import struct

  # Configure the serial port
ser = serial.Serial('/dev/serial0', 115200)  # Use the correct serial port for your Raspberry Pi

def send_motor_command(motor1_speed, motor2_speed):
    """
    Constructs and sends a CMD_SET_MOTORS command to the MCU.
    """
    command_id = 0x01
    payload = struct.pack('<hh', motor1_speed, motor2_speed)
    payload_length = len(payload)

      # Calculate checksum
    checksum = command_id ^ payload_length
    for byte in payload:
        checksum ^= byte

      # Construct the message
    message = bytearray([0xAA, command_id, payload_length])
    message.extend(payload)
    message.append(checksum)
    message.append(0x55)

      # Send the message
    ser.write(message)


if __name__ == "__main__":
    send_motor_command(0, 0)
