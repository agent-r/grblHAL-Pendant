#!/usr/bin/env python3

import socket
import requests
import threading

import time
import warnings
import sys, os
import serial
import serial.tools.list_ports
import json


status_timer = None
old_status = None

# print(sys.version)
## -------------------- SETUP -------------------


def connect_pendant():
    global ser
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if 'Silicon Labs' in str(p.manufacturer):
            print('Pendant ... OK')
            ser = serial.Serial(p.device, 128000, timeout=0.05)
            if ser.isOpen():
                ser.close()
            ser.open()
            ser.isOpen()
            return False
    print('Waiting for Pendant...')
    return True

def check_server():
    a_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    location = ("localhost", 8080)
    result_of_check = a_socket.connect_ex(location)
    if result_of_check == 0:
        print("bCNC Server ... OK")
        return False
    else:
        print("Waiting for bCNC Server...")
        return True
    a_socket.close()

## -------------------- SERVICES -------------------

def get_status():

    global status_timer
    global old_status

    try:
        status_long = requests.get('http://localhost:8080/state')
    except:
        print("bCNC Server lost...")
        while check_server() or connect_pendant():
            time.sleep(2)
        status_long = requests.get('http://localhost:8080/state')


    status_long_json = json.loads(status_long.text)

    status_short = { "state": status_long_json["state"], "wx": status_long_json["wx"], "wy": status_long_json["wy"], "wz": status_long_json["wz"] }
    status_short_json = json.dumps(status_short)

    if old_status != status_short:
        old_status = status_short
        print(status_long.text)
        try:
            ser.write(str(status_short).encode())
        except serial.SerialException:
            print("Pendant lost...")
            while check_server() or connect_pendant():
                time.sleep(1)


    while ser.in_waiting:  # Or: while ser.inWaiting():
        try:
            incoming = ser.readline().decode(errors='ignore')
        except:
            pass

        in_list = incoming.split(";")

        for i in in_list:
            # print(i)
            try:
                command_json = json.loads(i)
                try:
                    print("G: " + command_json["G"])
                    try:
                        requests.get("http://localhost:8080/send?gcode=" + command_json["G"])
                    except:
                        print("bCNC Server lost...")
                        while check_server() or connect_pendant():
                            time.sleep(2)
                except:
                    pass
                try:
                    print("C: " + command_json["C"])
                    try:
                        requests.get("http://localhost:8080/send?cmd=" + command_json["C"])
                    except:
                        print("bCNC Server lost...")
                        while check_server() or connect_pendant():
                            time.sleep(2)
                except:
                    pass
            except:
                pass


    status_timer = threading.Timer(.2, get_status)
    status_timer.start()



## -------------------- LOOP -------------------

def main():

    while check_server() or connect_pendant():
        time.sleep(2)

    status_timer = threading.Timer(.2, get_status)
    status_timer.start()
    input() # Don't exit the program


if __name__ == '__main__':

    main()
