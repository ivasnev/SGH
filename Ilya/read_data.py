import serial
from time import sleep
import json

port = 'COM5'
arduino = serial.Serial(port, 9600)
sleep(2)
for _ in range(3):
    json_inp = str(arduino.readline())
    print("startup", json_inp)
    sleep(1)

while 1:
    json_inp = str(arduino.readline())
    json_inp = json_inp[2:-5]
    dic = json.loads(json_inp)
    print(dic)
    sleep(1)

