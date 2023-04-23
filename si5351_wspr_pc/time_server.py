#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Эмулятор GPS (до некоторой степени, только $GPRMC и время)

import sys
import threading
import serial
import glob
import time

#запуск порта:
ports = glob.glob('/dev/ttyUSB0')
for port in ports:                     #хорошо, если он там один такой
	try:
		ser = serial.Serial(port,115200)
		print('ttyUSB' + port + ' Arduino connected')
	except:
		print('ttyUSBx Arduino connection error')
		raise SystemExit()
time.sleep(1) #дать порту время законнектиться

try:
    while True:
	#периодически посылаем время
	t = time.localtime()
	current_time = time.strftime("%H%M%S", t)
	print(current_time)
	ser.write('$GPRMC,' + current_time + '\n') #отправляем
	time.sleep(0.1) #пауза, чтобы не гнать зазря
except KeyboardInterrupt: #выходим по нажатию ^C
    pass

ser.close()
print('Bye!')
