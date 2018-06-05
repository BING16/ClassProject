//CHONG：
// Visual Micro is in vMicro>General>Tutorial Mode
//
/*
Name:       MotorArduinoProject.ino
Created:  2018/5/23 20:03:01
Author:     LAPTOP-EA0VE2G1\qbj

*蓝牙每次发送信号时第一个数与其他数分行？
*/

#include <SoftwareSerial.h>
#include <SPI.h>

// Define User Types below here or use a .h file
#define LED  11
SoftwareSerial BT(9, 10);  //新建对象，接收脚为9(对应蓝牙的T)，发送脚为10(对应蓝牙的R)
int LIGHT;
int temp = 0;
//

// Define Functions below here or use other .ino or cpp files

void LED_(int L) {
	L = 255 - L;
	analogWrite(LED, L);
}

//

// The setup() function runs once each time the micro-controller starts
void setup()
{
	// Open serial communications and wait for port to open:
	Serial.begin(38400);
	BT.begin(38400);  //设置波特率
	while (!Serial) {
		// wait for serial port to connect. Needed for native USB port only
	}

	pinMode(LED, OUTPUT);
	do {
		if (BT.available()) {
			char val = BT.read();
			//Serial.write(val+'0');
			if (val == 'K') {
				BT.write(val);
				Serial.println(val);
			}
			else if (val == 'Y') {
				Serial.println(val);
				break;
			}
			delay(500);
		}
	} while (1);
	while (BT.available()) {
		BT.read();
	}
	LED_(0);
}

// Add the main program code into the continuous loop() function
void loop()
{
	if (BT.available()) {
		char val = BT.read();
		//Serial.println(val);
		if (val == '-') {
			LED_(0);
			temp = 0;
			LIGHT = 0;
			Serial.println(0);
		}
		else {
			LIGHT = LIGHT * 10 + (val);
			temp = temp + 1;
			if (temp == 3) {
				LED_(LIGHT);
				Serial.println(LIGHT);
			}
		}
	}

}

