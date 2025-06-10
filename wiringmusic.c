#include <wiringPi.h>
#include <softTone.h>

#define SPKR	6
#define TOTAL	32
/*
int notes[] = {
	391, 391, 440, 440, 391, 391, 329.63, 329.63, \
	391, 391, 329.63, 329.63, 293.66, 293.66, 293.66, 0, \
	391, 391, 440, 440, 391, 391, 329.63, 329.63, \
	391, 329.63, 293.66, 329.63, 261.63, 261.63, 261.63, 0
};
*/

int notes[] = {
    294, 440, 494, 370, 392, 294, 392, 440, 0,\
    494, 440, 392, 370, 330, 294, 330, 370, 0,\
    392, 370, 330, 294, 554, 494, 554, 587,\
    659, 587, 554, 494, 440, 392, 370, 330, 0,0,\
    294, 330, 370, 392, 440, 494, 554, 587
};

int musicPlay() {
	int i;
	softToneCreate(SPKR);
	for (i=0; i<TOTAL; ++i) {
		softToneWrite(SPKR, notes[i]);
		delay(200);
	}
	return 0;
}

int main() {
	wiringPiSetup();
	musicPlay();
	return 0;
}
