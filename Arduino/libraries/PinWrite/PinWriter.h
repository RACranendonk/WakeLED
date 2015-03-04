/*==================
Robert Cranendonk
WakeLED. Personal Arduino project
21-02-2014
====================*/

#ifndef PINWRITER_H
#define PINWRITER_H

#include "Arduino.h"

class PinWriter
{
private:
	int _redPin;
	int _greenPin;
	int _bluePin;
public:
	PinWriter(int redPin, int greenPin, int bluePin);
	void write(int red, int green, int blue);
};

#endif