all: imu

imu:
	g++ -Wall -g -O2 -c -o I2Cdev.o I2Cdev.cpp
	g++ -Wall -g -O2 -c -o MPU6050.o MPU6050.cpp
	g++ -Wall -g -O2 -c -o dmp.o dmp.cpp
	g++ -o dmp I2Cdev.o MPU6050.o dmp.o -lm
