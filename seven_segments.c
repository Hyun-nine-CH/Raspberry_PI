#include <wiringPi.h>
#include <stdio.h>

#define a 15
#define b 16
#define c 1
#define d 4
#define e 8
#define f 5
#define g 7

int seven_seg() {
	
	pinMode(a, OUTPUT);	
	pinMode(b, OUTPUT);
	pinMode(c, OUTPUT);
	pinMode(d, OUTPUT);
	pinMode(e, OUTPUT);
	pinMode(f, OUTPUT);
	pinMode(g, OUTPUT);

	delay(1000);
//0
	digitalWrite(a, HIGH);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, HIGH);
	digitalWrite(e, HIGH);
	digitalWrite(f, HIGH);
	digitalWrite(g, LOW);

	delay(1000);
//1
	digitalWrite(a, LOW);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, LOW);
	digitalWrite(e, LOW);
	digitalWrite(f, LOW);
	digitalWrite(g, LOW);

	delay(1000);
//2
	digitalWrite(a, HIGH);
	digitalWrite(b, HIGH);
	digitalWrite(c, LOW);
	digitalWrite(d, HIGH);
	digitalWrite(e, HIGH);
	digitalWrite(f, LOW);
	digitalWrite(g, HIGH);

	delay(1000);
//3
	digitalWrite(a, HIGH);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, HIGH);
	digitalWrite(e, LOW);
	digitalWrite(f, LOW);
	digitalWrite(g, HIGH);

	delay(1000);
//4
	digitalWrite(a, LOW);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, LOW);
	digitalWrite(e, LOW);
	digitalWrite(f, HIGH);
	digitalWrite(g, HIGH);

	delay(1000);
//5
	digitalWrite(a, HIGH);
	digitalWrite(b, LOW);
	digitalWrite(c, HIGH);
	digitalWrite(d, HIGH);
	digitalWrite(e, LOW);
	digitalWrite(f, HIGH);
	digitalWrite(g, HIGH);

	delay(1000);
//6
	digitalWrite(a, HIGH);
	digitalWrite(b, LOW);
	digitalWrite(c, HIGH);
	digitalWrite(d, HIGH);
	digitalWrite(e, HIGH);
	digitalWrite(f, HIGH);
	digitalWrite(g, HIGH);

	delay(1000);
//7
	digitalWrite(a, HIGH);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, LOW);
	digitalWrite(e, LOW);
	digitalWrite(f, LOW);
	digitalWrite(g, LOW);

	delay(1000);
//8
	digitalWrite(a, HIGH);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, HIGH);
	digitalWrite(e, HIGH);
	digitalWrite(f, HIGH);
	digitalWrite(g, HIGH);

	delay(1000);
//9
	digitalWrite(a, HIGH);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, LOW);
	digitalWrite(e, LOW);
	digitalWrite(f, HIGH);
	digitalWrite(g, HIGH);

	delay(1000);
}

int main() {
	wiringPiSetup();
	seven_seg();
	return 0;
}

