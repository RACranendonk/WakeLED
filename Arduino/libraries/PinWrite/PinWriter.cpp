/*==================
Robert Cranendonk
WakeLED. Personal Arduino project
21-02-2014
====================*/

#include "PinWriter.h"

PinWriter::PinWriter(int redPin, int greenPin, int bluePin)
{
	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
	_redPin = redPin;
	_greenPin = greenPin;
	_bluePin = bluePin;
}

void PinWriter::write(int red, int green, int blue)
{
      analogWrite(_redPin, red);
      analogWrite(_greenPin, green);
      analogWrite(_bluePin, blue);
}

