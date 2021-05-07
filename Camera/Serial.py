import serial
import time
arduino = serial.Serial(port='COM6', baudrate=115200, timeout=.1)

def write_read(x):
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)
    if arduino.in_waiting:
        data = arduino.readline()
        print('Received: ', data)
    return

def printBytes(toPrint):
    print('length: ', len(toPrint))
    for i in range(len(toPrint)):
        print(hex(toPrint[i]), end=' ')
    print()
