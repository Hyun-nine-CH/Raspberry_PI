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
	
	pinMode(a, INPUT);	
	pinMode(b, INPUT);
	pinMode(c, INPUT);
	pinMode(d, INPUT);
	pinMode(e, INPUT);
	pinMode(f, INPUT);
	pinMode(g, INPUT);

//0
	digitalWrite(a, HIGH);
	digitalWrite(b, HIGH);
	digitalWrite(c, HIGH);
	digitalWrite(d, HIGH);
	digitalWrite(e, HIGH);
	digitalWrite(f, HIGH);
	digitalWrite(g, LOW);



}
