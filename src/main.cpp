#include <Arduino.h>
#include <TimerOne.h>

#define PIN_MEMW 21
#define PIN_MEMR 20
#define PIN_IOW 19
#define PIN_IOR 18

#define PIN_RESET 17
#define PIN_AEN 16
#define PIN_ALE 15
#define PIN_IOCHRDY 14
#define PIN_REFRESH 2

#define PORT_ADDR0 PORTA  // [A0:A7]
#define PORT_ADDR8 PORTC  // [A8:A15]
#define PORT_ADDR16 PORTL // [A16:A19]
#define PORT_DATA PORTK
#define PIN_DATA PINK

#define DDR_ADDR0 DDRA
#define DDR_ADDR8 DDRC
#define DDR_ADDR16 DDRL
#define DDR_DATA DDRK
volatile uint8_t refreshState = 1;

void ioWrite(uint32_t address, uint8_t data)
{
	PORT_ADDR0 = address & 0xFF;
	PORT_ADDR8 = (address >> 8) & 0xFF;
	PORT_ADDR16 = (address >> 16) & 0xFF;

	//digitalWrite(PIN_ALE, HIGH);
	//digitalWrite(PIN_ALE, LOW);

	PORT_DATA = data;
	DDR_DATA = 0xFF;

	digitalWrite(PIN_IOW, LOW);
	//delay(1);

	while (!digitalRead(PIN_IOCHRDY))
	{
	}

	digitalWrite(PIN_IOW, HIGH);
	//delay(1);

	DDR_DATA = 0x00;
}

uint8_t ioRead(uint32_t address)
{
	uint8_t data = 0x00;

	PORT_ADDR0 = address & 0xFF;
	PORT_ADDR8 = (address >> 8) & 0xFF;
	PORT_ADDR16 = (address >> 16) & 0xFF;

	//igitalWrite(PIN_ALE, HIGH);
	//digitalWrite(PIN_ALE, LOW);

	PORT_DATA = 0xFF; // pullup data bus
	DDR_DATA = 0x00;
	digitalWrite(PIN_IOR, LOW);
	//delay(1);

	while (!digitalRead(PIN_IOCHRDY))
	{
	}

	data = PIN_DATA;

	digitalWrite(PIN_IOR, HIGH);
	//delay(1);

	return data;
}

void memWrite(uint32_t address, uint8_t data)
{
	PORT_ADDR0 = address & 0xFF;
	PORT_ADDR8 = (address >> 8) & 0xFF;
	PORT_ADDR16 = (address >> 16) & 0xFF;

	//digitalWrite(PIN_ALE, HIGH);

	//digitalWrite(PIN_ALE, LOW);

	PORT_DATA = data;
	DDR_DATA = 0xFF;

	digitalWrite(PIN_MEMW, LOW);
	delayMicroseconds(10);

	while (!digitalRead(PIN_IOCHRDY))
	{
	}

	digitalWrite(PIN_MEMW, HIGH);
	delayMicroseconds(10);

	DDR_DATA = 0x00;
}

uint8_t memRead(uint32_t address)
{
	uint8_t data = 0x00;


	PORT_ADDR0 = address & 0xFF;
	PORT_ADDR8 = (address >> 8) & 0xFF;
	PORT_ADDR16 = (address >> 16) & 0xFF;

	//digitalWrite(PIN_ALE, HIGH);
	//digitalWrite(PIN_ALE, LOW);

	PORT_DATA = 0xFF; // pullup data bus
	DDR_DATA = 0x00;
	digitalWrite(PIN_MEMR, LOW);
	delayMicroseconds(10);

	while (!digitalRead(PIN_IOCHRDY))
	{
	}

	data = PIN_DATA;

	digitalWrite(PIN_MEMR, HIGH);
	delayMicroseconds(10);

	return data;
}

void ioIndexedWrite(uint32_t address, uint16_t data)
{
	ioWrite(address, data & 0xFF);
	ioWrite(address + 1, (data >> 8) & 0xFF);
}
uint8_t ioIndexedRead(uint32_t address, uint8_t index)
{
	ioWrite(address, index);
	return ioRead(address + 1);
}
/*void ioIndexedWrite(uint32_t address, uint8_t index, uint8_t data)
{
	ioWrite(address, index);
	ioWrite(address + 1, data);
}
uint8_t ioIndexedRead(uint32_t address, uint8_t index)
{
	ioWrite(address, index);
	return ioRead(address + 1);
}*/

void refreshDRAM()
{
	refreshState = 0;
	digitalWrite(PIN_REFRESH, 0);
	delayMicroseconds(15);
	digitalWrite(PIN_REFRESH, 1);
	refreshState = 1;
}

void setup()
{
	Serial.begin(1000000);

	PORT_ADDR0 = 0x00;
	PORT_ADDR8 = 0x00;
	PORT_ADDR16 = 0x00;

	DDR_ADDR0 = 0xFF;  // [A0:A7]
	DDR_ADDR8 = 0xFF;  // [A8:A15]
	DDR_ADDR16 = 0xFF; // [A16:A19]

	digitalWrite(PIN_MEMW, HIGH);
	digitalWrite(PIN_MEMR, HIGH);
	digitalWrite(PIN_IOW, HIGH);
	digitalWrite(PIN_IOR, HIGH);
	digitalWrite(PIN_REFRESH, HIGH);

	digitalWrite(PIN_AEN, LOW);
	digitalWrite(PIN_ALE, HIGH);

	digitalWrite(PIN_IOCHRDY, HIGH); // Pull-UP

	pinMode(PIN_MEMW, OUTPUT);
	pinMode(PIN_MEMR, OUTPUT);
	pinMode(PIN_IOW, OUTPUT);
	pinMode(PIN_IOR, OUTPUT);
	pinMode(PIN_REFRESH, OUTPUT);

	pinMode(PIN_ALE, OUTPUT);
	pinMode(PIN_AEN, OUTPUT);

	digitalWrite(PIN_RESET, HIGH);
	pinMode(PIN_RESET, OUTPUT);

	delay(100);
	digitalWrite(PIN_RESET, LOW);
	delay(1000);

	PORT_ADDR0 = 0x00;
	PORT_ADDR8 = 0x00;
	PORT_ADDR16 = 0x00;

	DDR_ADDR0 = 0xFF;  // [A0:A7]
	DDR_ADDR8 = 0xFF;  // [A8:A15]
	DDR_ADDR16 = 0xFF; // [A16:A19]

	//Timer1.initialize(15000);
	//Timer1.attachInterrupt(refreshDRAM);

	Serial.print("R");
	Serial.print('\n');

	
}

uint8_t command;
uint32_t address;
uint8_t data;

void loop()
{
	if (Serial.available())
	{
		command = Serial.read();
		if (command == 'i')
		{
			while (Serial.available() < 2) {}
			address = (Serial.read() << 8);
			address |= Serial.read();
			Serial.write(ioRead(address));
		}

		if (command == 'o')
		{
			while (Serial.available() < 3)	{}
			address = (Serial.read() << 8);
			address |= Serial.read();
			data = Serial.read();

			ioWrite(address, data);
			Serial.write('o');
		}

		if (command == 'r')
		{
			while (Serial.available() < 3)	{}
			address = ((uint32_t) Serial.read() << 16UL);
			address |= (Serial.read() << 8);
			address |= Serial.read();
			Serial.write(memRead(address));
		}

		if (command == 'w')
		{
			while (Serial.available() < 4)	{}
			address = ((uint32_t)Serial.read() << 16UL);
			address |= (Serial.read() << 8);
			address |= Serial.read();
			data = Serial.read();

			memWrite(address, data);
			Serial.write('w');
		}
	}
	//digitalWrite(PIN_REFRESH, 0);
	//delayMicroseconds(15);
	//digitalWrite(PIN_REFRESH, 1);
}