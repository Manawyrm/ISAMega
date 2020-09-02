#include <Arduino.h>

#define PIN_MEMW 21
#define PIN_MEMR 20
#define PIN_IOW 19
#define PIN_IOR 18

#define PIN_RESET 17
#define PIN_AEN 16
#define PIN_ALE 15
#define PIN_IOCHRDY 14

#define PORT_ADDR0 PORTA  // [A0:A7]
#define PORT_ADDR8 PORTC  // [A8:A15]
#define PORT_ADDR16 PORTL // [A16:A19]
#define PORT_DATA PORTK
#define PIN_DATA PINK

#define DDR_ADDR0 DDRA
#define DDR_ADDR8 DDRC
#define DDR_ADDR16 DDRL
#define DDR_DATA DDRK

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
	delay(1);

	while (!digitalRead(PIN_IOCHRDY))
	{
	}

	digitalWrite(PIN_IOW, HIGH);
	delay(1);

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
	delay(1);

	while (!digitalRead(PIN_IOCHRDY))
	{
	}

	data = PIN_DATA;

	digitalWrite(PIN_IOR, HIGH);
	delay(1);

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

void ioIndexedWrite(uint32_t address, uint8_t index, uint8_t data)
{
	ioWrite(address, index);
	ioWrite(address + 1, data);
}
uint8_t ioIndexedRead(uint32_t address, uint8_t index)
{
	ioWrite(address, index);
	return ioRead(address + 1);
}

void setup()
{
	Serial.begin(115200);

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

	digitalWrite(PIN_RESET, HIGH);
	digitalWrite(PIN_AEN, LOW);
	digitalWrite(PIN_ALE, HIGH);

	digitalWrite(PIN_IOCHRDY, HIGH); // Pull-UP

	pinMode(PIN_MEMW, OUTPUT);
	pinMode(PIN_MEMR, OUTPUT);
	pinMode(PIN_IOW, OUTPUT);
	pinMode(PIN_IOR, OUTPUT);

	pinMode(PIN_RESET, OUTPUT);
	pinMode(PIN_ALE, OUTPUT);
	pinMode(PIN_AEN, OUTPUT);


	delay(100);
	digitalWrite(PIN_RESET, LOW);
	delay(1000);

	Serial.print("R");
	Serial.print('\n');

	PORT_ADDR0 = 0x00;
	PORT_ADDR8 = 0x00;
	PORT_ADDR16 = 0x00;

	DDR_ADDR0 = 0xFF;  // [A0:A7]
	DDR_ADDR8 = 0xFF;  // [A8:A15]
	DDR_ADDR16 = 0xFF; // [A16:A19]
}

char textBuf[32];

String command;
String address;
String data;

uint16_t currentPin = 0;

void loop()
{
	command = Serial.readStringUntil('\n');
	if (command.length() != 0)
	{
		if (command[0] == 'i')
		{
			address = command.substring(1, 5);
			Serial.print("i");
			Serial.print(ioRead((uint32_t)strtol(&address[0], NULL, 16)), HEX);
			Serial.print('\n');
		}

		if (command[0] == 'o')
		{
			address = command.substring(1, 5);
			data = command.substring(5, 7);

			ioWrite((uint32_t)strtol(&address[0], NULL, 16), (uint8_t)strtol(&data[0], NULL, 16));

			Serial.print("o\n");
		}

		if (command[0] == 'r')
		{
			address = command.substring(1, 7);
			Serial.print("r");
			Serial.print(memRead((uint32_t)strtol(&address[0], NULL, 16)), HEX);
			Serial.print('\n');
		}

		if (command[0] == 'w')
		{
			address = command.substring(1, 7);
			data = command.substring(7, 9);

			memWrite((uint32_t)strtol(&address[0], NULL, 16), (uint8_t)strtol(&data[0], NULL, 16));

			Serial.print("w\n");
		}
	}
}