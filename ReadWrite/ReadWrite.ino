// Visual Micro is in vMicro>General>Tutorial Mode
//
/*
Name:       MotorArduinoProject.ino
Created:  2018/5/23 20:03:01
Author:     LAPTOP-EA0VE2G1\qbj

* SD card attached to SPI bus as follows:
** MOSI - pin 11
** MISO - pin 12
** CLK - pin 13x
** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

*主蓝牙使用HC-05
*从蓝牙使用HC-06
*/

#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

// Define User Types below here or use a .h file
enum Dimension { X = 0, Y = 1 };
enum Direction { FRONT = 0, BACK = 1 };
SoftwareSerial BT(9, 10);  //新建对象，接收脚为9(对应蓝牙的T)，发送脚为10(对应蓝牙的R)

File myFile;
int N = 145;
int STEPXX = 12800;
int STEPYY = 12000;
int STEPX = STEPXX / N *10;    //每布的距离，越大越长。X轴大概640对应200像素
int STEPY = STEPYY / N *10; //Y轴大概600对应200像素
int V = 90;    //速度相关，越小越快。X轴100对应2.306s走80mm；200对应2.593走80mm；60对应92mm/s
int SLEEP = 1000;
int W = 100;
int H = 143;
int x = 0;
int y = 0;
//

// Define Function Prototypes that use User Types below here or use a .h file
void Step_(Dimension D, Direction DIR);
void Move_();
void Move_B();
void OpenFile_(String Name);
void CloseFile_();
int ReadFile_(int x, int y);
void SendInfo_(int INFO);
//

//int IMG_() {
//  return img[y - 1][x - 1];
//}

// Define Functions below here or use other .ino or cpp files
void Step_(Dimension D, Direction DIR) {      //0-X 1-Y, 0-F 1-B
	digitalWrite(5, LOW);
	digitalWrite(6, LOW);

	int NDIR;
	int NSTEP;
	int STEP;
	switch (D) {
	case X:
		NSTEP = 5;
		NDIR = 2;
		STEP = STEPX;
		break;
	case Y:
		NSTEP = 6;
		NDIR = 3;
		STEP = STEPY;
		break;
	}

	if (DIR == FRONT) {
		digitalWrite(NSTEP, HIGH);
		for (int i = 0; i < STEP; i++) {
			digitalWrite(NDIR, HIGH);
			delayMicroseconds(V);
			digitalWrite(NDIR, LOW);
			delayMicroseconds(V);
		}
		digitalWrite(NSTEP, LOW);
	}
	else if (DIR == BACK) {
		digitalWrite(NSTEP, LOW);
		for (int i = 0; i < STEP; i++) {
			digitalWrite(NDIR, LOW);
			delayMicroseconds(V);
			digitalWrite(NDIR, HIGH);
			delayMicroseconds(V);
		}
		digitalWrite(NSTEP, HIGH);
	}
}

void Move_() {
	delay(5000);
	Serial.println("Move");
	int L = 0;
	for (y = 1; y <= H;) {
		for (x = 1; x <= W; ) {
			L = ReadFile_(x, y);
			//L = IMG_();
			SendInfo_(L);
			Serial.println(L);
			delay(SLEEP);
			SendInfo_(0);
			Step_((Dimension)X, (Direction)FRONT);
			x++;
		}
		Step_((Dimension)Y, (Direction)FRONT);
		y++;
		if (y > H) {
			break;
		}
		for (x = W; x > 0;) {
			Step_((Dimension)X, (Direction)BACK);
			x--;
		}
	}
	CloseFile_();
}

void Move_B() {
	for (; x > 1; x--) {
		Step_(X, BACK);
	}
	for (; y > 1; y--) {
		Step_(Y, BACK);
	}
}

void OpenFile_(String Name) {
	Serial.print("Initializing SD card...");
	W = 0;
	H = 0;

	if (!SD.begin(4)) {
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");

	// open the file. note that only one file can be open at a time,
	// so you have to close this one before opening another.
	// re-open the file for reading:
	myFile = SD.open(Name);
	if (myFile) {
		// read from the file until there's nothing else in it:
		while (myFile.available()) {
			char c = myFile.read();      //先读取H，后读取W
			if (c == '\n' || c == ',')
				break;
			else if (c >= '0'&&c <= '9') {      // ',' = -4, '\n' = -38
				H = H * 10 + (c - '0');
			}
		}
		while (myFile.available()) {
			char c = myFile.read();
			if (c == '\n' || c == ',')
				break;
			else if (c >= '0'&&c <= '9') {      // ',' = -4, '\n' = -38
				W = W * 10 + (c - '0');
			}
			// close the file:
			//myFile.close();
		}
	}

	if (W <= H)
		N = H + 3;
	else
		N = W + 3;

	Serial.println(W);
	Serial.println(H);
	STEPX = STEPXX / N *10;    //每布的距离，越大越长。X轴大概640对应200像素
	STEPY = STEPYY / N *10; //Y轴大概600对应200像素 
}

void CloseFile_() {
	myFile.close();
}

int ReadFile_(int x, int y) {
	int LIGHT = -1;
	while (myFile.available()) {
		char c = myFile.read();
		if (c < '0' || c > '9') {
			if (LIGHT != -1)
				break;
		}
		else {      // ',' = -4, '\n' = -38
			if (LIGHT == -1)
				LIGHT = 0;
			LIGHT = LIGHT * 10 + (c - '0');
		}
	}
	return LIGHT;
}

void SendInfo_(int INFO) {
	if (INFO == 0) {   //表示灭灯
		BT.write('-');
	}
	else/* if (INFO >= 0 && INFO <= 255) */ {   //亮灯
		BT.write(char(INFO / 100));
		INFO = INFO % 100;
		BT.write(char(INFO / 10));
		INFO = INFO % 10;
		BT.write(char(INFO / 1));
	}
}
//

// The setup() function runs once each time the micro-controller starts
void setup()
{
	// Open serial communications and wait for port to open:
	BT.begin(38400);  //设置波特率
	Serial.begin(38400);//这里应该和你的模块通信波特率一致

	while (!Serial) {
		// wait for serial port to connect. Needed for native USB port only
	}
	OpenFile_("1.txt");
	Serial.println(W);
	Serial.print(H);

	pinMode(2, OUTPUT);      //X.step
	pinMode(5, OUTPUT);      //XDIR
	pinMode(3, OUTPUT);      //Y.step
	pinMode(6, OUTPUT);      //YDIR
	pinMode(8, OUTPUT);      //LOW步进电机使能

	digitalWrite(8, LOW);
	digitalWrite(6, LOW);
	digitalWrite(5, LOW);

	do {
		BT.write('K');
		Serial.println("开始确认蓝牙配对情况");
		delay(500);
		if (BT.available()) {
			char val = BT.read();
			if (val == 'K') {
				BT.write("Y");
				Serial.println("接收到蓝牙信息");
				break;
			}
		}
	} while (1);
	while (BT.available()) {
		BT.read();
	}
	Serial.println("蓝牙完成连接");
}

// Add the main program code into the continuous loop() function
void loop()
{
	////delay(5000);
	//if (Serial.available()) {
	//  char a = Serial.read();
	//  BT.write(a);
	//}

	////如果接收到蓝牙模块的数据，输出到屏幕
	//if (BT.available()) {
	//  char val = BT.read();
	//  Serial.print(val);
	//}

	if (x == 0 && y == 0)
		Move_();


}


