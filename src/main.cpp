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

uint8_t	ioRead(uint32_t address)
{
	uint8_t data = 0x00;

	PORT_ADDR0 = address & 0xFF;
	PORT_ADDR8 = (address >> 8) & 0xFF;
	PORT_ADDR16 = (address >> 16) & 0xFF;

	digitalWrite(PIN_ALE, HIGH);
	digitalWrite(PIN_ALE, LOW);

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

	digitalWrite(PIN_ALE, HIGH);
	digitalWrite(PIN_ALE, LOW);

	PORT_DATA = data;
	DDR_DATA = 0xFF;

	digitalWrite(PIN_MEMR, LOW);
	delayMicroseconds(10);

	while (!digitalRead(PIN_IOCHRDY))
	{
	}
	
	digitalWrite(PIN_MEMR, HIGH);
	delayMicroseconds(10);

	DDR_DATA = 0x00;
}

uint8_t memRead(uint32_t address)
{
	uint8_t data = 0x00;

	PORT_ADDR0 = address & 0xFF;
	PORT_ADDR8 = (address >> 8) & 0xFF;
	PORT_ADDR16 = (address >> 16) & 0xFF;

	digitalWrite(PIN_ALE, HIGH);
	digitalWrite(PIN_ALE, LOW);

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

void IoPortOutB(uint32_t address, uint8_t data)
{
	ioWrite(address, data); 
}
uint8_t IoPortInB(uint32_t address)
{
	return ioRead(address);
}

uint8_t	VgaIoReadIx(uint32_t address, uint8_t index)
{
	return ioIndexedRead(address, index);
}
void VgaIoWriteIx(uint32_t address, uint16_t data)
{
	ioIndexedWrite(address, (data >> 8) & 0xFF, data & 0xFF);
}

//*********************************************************************
void sub_2EA(void)
{
	VgaIoWriteIx(0x3C4, 0x000B); // Set oldmode
}
//*********************************************************************
uint8_t sub_292(void)
{
	sub_2EA();						   // Set oldmode
	return (VgaIoReadIx(0x3C4, 0x0D)); // Old mode 0x3C4.0x0D read
}
//*********************************************************************

uint8_t sub_4D9(void)
{
	uint8_t al;

	al = sub_292() & 0x0E;
	if (al != 0x0C)
		return (1); // return no zero
	al = IoPortInB(0x3CC) & 0x67;
	if (al != 0x67)
		return (1); // return no zero
	return (0);
}

//*********************************************************************
uint8_t sub_26A()
{
	VgaIoReadIx(0x3C4, 0x0B); // New mode set
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) | 0x80) ^ 2) << 8) + 0x0E);
	return (VgaIoReadIx(0x3C4, 0x0C));
}
//*********************************************************************
void sub_179(void)
{
	// SP BP manipulate
	VgaIoWriteIx(0x3C4, (((sub_26A() | 0x42) & 0xFE) << 8) + 0x0C);
	VgaIoWriteIx(0x3C4, ((VgaIoReadIx(0x3C4, 0x0F) | 0x80) << 8) + 0x0F);
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) & 0x7F) ^ 2) << 8) + 0x0E);
}
//*********************************************************************
void sub_51A(void)
{
	uint8_t al, bh;
	bh = (sub_26A() | 0x80) & 0xFE;
	VgaIoWriteIx(0x3C4, 0x2407);
	IoPortOutB(0x3C2, 0x01);
	if (!((al = VgaIoReadIx(0x3D4, 0x28)) & 0x0C))
	{
		al |= 0x04;
		VgaIoWriteIx(0x3D4, (al << 8) + 0x28);
	}
	VgaIoWriteIx(0x3C4, ((VgaIoReadIx(0x3C4, 0x0F) & 0x7F) << 8) + 0x0F);
	VgaIoWriteIx(0x3C4, (bh << 8) + 0x0C);
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) & 0x7F) ^ 2) << 8) + 0x0E);
	if (VgaIoReadIx(0x3C4, 0x0F) & 0x08)
	{
		sub_179();
	}
	sub_2EA(); // Old mode
	VgaIoWriteIx(0x3C4, 0x200D);
	VgaIoWriteIx(0x3C4, 0xA00E);
	VgaIoReadIx(0x3C4, 0x0B); // New mode
	VgaIoWriteIx(0x3C4, 0x020E);
	if (!((al = VgaIoReadIx(0x3CE, 0x06)) & 0x0C))
	{
		VgaIoWriteIx(0x3CE, (((al & 0xF3) | 0x04) << 8) + 0x06);
	}
	VgaIoWriteIx(0x3C4, 0x000D);
	al = VgaIoReadIx(0x3D4, 0x1E);
	VgaIoWriteIx(0x3D4, 0x001E);
}
//*********************************************************************
void TR9000i_Init(void)
{
	uint16_t i = 0;
	IoPortOutB(0x03C3, 0x00);
	if (!sub_4D9())
		goto loc_E88;
	do
	{
		IoPortOutB(0x3C9, 0x00);
		i++;
	} while (i < 768);
	IoPortOutB(0x3C2, 0x23);
	sub_51A();
	//  IoPortOutB(0x3D4,0x1F);
	//  IoPortOutB(0x3D5,0x81);

	//  IoPortOutB(0x3D4,0x25);
	//  IoPortOutB(0x3D5,0xFF);

	// if(((sub_292()&0x0E)==0x0C)&& IoPortInB(0x3CC)==0x67));

loc_E88:
	{}
}
//*********************************************************************
//*********************************************************************
//*********************************************************************

uint8_t sub_511(void)
{
	VgaIoWriteIx(0x3C4, 0x000B); // Old mode set
	if ((VgaIoReadIx(0x3C4, 0x0D) & 0x0E) != 0x0C)
		return (1);
	if ((IoPortInB(0x3CC) & 0x67) != 0x67)
		return (1);
	return (0);
}
//*********************************************************************
void sub_522(void)
{ // sub_170
	uint8_t al, bh;

	VgaIoReadIx(0x3C4, 0x0B); // New mode set
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) | 0x80) ^ 2) << 8) + 0x0E);
	bh = (VgaIoReadIx(0x3C4, 0x0C) & 0xFE) | 0x80;
	if (VgaIoReadIx(0x3D4, 0x28) & 0x01)
		goto loc_1B0;
	VgaIoWriteIx(0x3C2, 0x01); // select 0x3Dx addr (original 0x01)
	if (((al = VgaIoReadIx(0x3D4, 0x28)) & 0x0C))
		goto loc_198;
	al |= 0x04;
	VgaIoWriteIx(0x3D4, (al << 8) + 0x28);
loc_198:
	VgaIoWriteIx(0x3C4, ((VgaIoReadIx(0x3C4, 0x0F) & 0x7F) << 8) + 0x0F);
	VgaIoReadIx(0x3C4, 0x0C);
	VgaIoWriteIx(0x3C4, (bh << 8) + 0x0C);
	if (al & 0x01)
		goto loc_1B0;
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) & 0x7F) ^ 2) << 8) + 0x0E);
	if (!(VgaIoReadIx(0x3C4, 0x0F) & 0x08))
		goto loc_1B0;
	VgaIoReadIx(0x3C4, 0x0B); // New mode set
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) | 0x80) ^ 2) << 8) + 0x0E);
	al = (VgaIoReadIx(0x3C4, 0x0C) & 0xFE) | 0x80;
	al &= 0xCE;
	// al |= bh;
	VgaIoWriteIx(0x3C4, (al << 8) + 0x0C);
	VgaIoWriteIx(0x3CE, 0x000F);
	VgaIoWriteIx(0x3C4, ((VgaIoReadIx(0x3C4, 0x0F) | 0x80) << 8) + 0x0F);
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) & 0x7F) ^ 0x02) << 8) + 0x0E);

loc_1B0:
	VgaIoWriteIx(0x3C4, 0x000B); // Old mode
	VgaIoWriteIx(0x3C4, 0x200D);
	VgaIoWriteIx(0x3C4, 0xA00E);
	VgaIoReadIx(0x3C4, 0x0B); // New mode set
	VgaIoWriteIx(0x3C4, 0x020E);
	al = VgaIoReadIx(0x3CE, 0x06);
	if (!al)
	{
		al &= 0xF3;
		al |= 4;
		VgaIoWriteIx(0x3CE, (al << 8) + 0x06);
	}
	VgaIoWriteIx(0x3C4, 0x000D);
	VgaIoReadIx(0x3D4, 0x1E);
	VgaIoWriteIx(0x3D4, 0x001E);
	VgaIoWriteIx(0x3D4, 0x1C20);
	VgaIoWriteIx(0x3D4, 0x0029);
}
//*********************************************************************
void TR8900CL_Init(void)
{
	// Int10H  ax=0x1201 bl=0x32 Video addressing disable
	// sub_a60 subsystem enable
	//  IoPortOutB(0x03C3,0x00);      // Subsystem enable already executed
	if (!sub_511())
		goto loc_ACF;
	sub_522();
	IoPortOutB(0x3C2, 0x23);
	if (sub_511())
		goto loc_ACF;
	IoPortInB(0x3DA);
	IoPortOutB(0x3C0, 0x20);
	IoPortInB(0x3DA);
	VgaIoWriteIx(0x3D4, ((VgaIoReadIx(0x3D4, 0x20) & 0xDF) << 8) + 0x20);
loc_ACF:
	IoPortOutB(0x3D8, 0x00);
	VgaIoWriteIx(0x3D4, 0x1023);
}

//*********************************************************************
//*********************************************************************
//*********************************************************************
void sub_5A6(void)
{ // call sub_16E()

	uint8_t al, bh;

	VgaIoReadIx(0x3C4, 0x0B); // New mode set
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) | 0x80) ^ 2) << 8) + 0x0E);
	bh = (VgaIoReadIx(0x3C4, 0x0C) & 0xFE) | 0x80;
	if (VgaIoReadIx(0x3C4, 0x0B) == 0x53) // New mode set
	{
		VgaIoReadIx(0x3D4, 0x29);
		VgaIoWriteIx(0x3D4, 0x4429);
		VgaIoWriteIx(0x3D4, 0x032B);
		VgaIoWriteIx(0x3D4, 0x3D2C);
		VgaIoWriteIx(0x3D4, 0x2725);
	}
	if (!VgaIoReadIx(0x3D4, 0x28) & 1)
		goto loc_1CA;
	bh &= 0xCE;
	bh |= 0x80;
	VgaIoWriteIx(0x3C2, 0x01); // select 0x3Dx addr (original 0x01)
	if (((al = VgaIoReadIx(0x3D4, 0x28)) & 0x0C))
		goto loc_1B2;
	al |= 0x04;
	VgaIoWriteIx(0x3D4, (al << 8) + 0x28);
loc_1B2:
	VgaIoWriteIx(0x3C4, ((VgaIoReadIx(0x3C4, 0x0F) & 0x7F) << 8) + 0x0F);
	VgaIoReadIx(0x3C4, 0x0C);
	VgaIoWriteIx(0x3C4, (bh << 8) + 0x0C);
	// if(al&0x01) goto loc_1CA;
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) & 0x7F) ^ 2) << 8) + 0x0E);
	if (!(VgaIoReadIx(0x3C4, 0x0F) & 0x08))
		goto loc_1CA;
	VgaIoReadIx(0x3C4, 0x0B); // New mode set
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) | 0x80) ^ 2) << 8) + 0x0E);
	al = (VgaIoReadIx(0x3C4, 0x0C) & 0xFE) | 0x80;
	al &= 0xFE;
	// al |= bh;
	VgaIoWriteIx(0x3C4, (al << 8) + 0x0C);
	VgaIoWriteIx(0x3CE, 0x000F);
	al = VgaIoReadIx(0x3C4, 0x0C); // removable testing
	VgaIoWriteIx(0x3C4, ((VgaIoReadIx(0x3C4, 0x0F) | 0x80) << 8) + 0x0F);

	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) & 0x7F) ^ 0x02) << 8) + 0x0E);
loc_1CA:
	VgaIoWriteIx(0x3C4, 0x000B); // Old mode
	VgaIoWriteIx(0x3C4, 0x200D);
	VgaIoWriteIx(0x3C4, 0xA00E);
	VgaIoReadIx(0x3C4, 0x0B); // New mode set
	VgaIoWriteIx(0x3C4, 0x020E);
	al = VgaIoReadIx(0x3CE, 0x06); // atvizsgálni
	if (!al)
	{
		al &= 0xF3;
		al |= 4;
		VgaIoWriteIx(0x3CE, (al << 8) + 0x06);
	}
	VgaIoWriteIx(0x3C4, 0x000D);
	VgaIoReadIx(0x3D4, 0x1E);
	VgaIoWriteIx(0x3D4, 0x001E);
	if (VgaIoReadIx(0x3C4, 0x0B) == 0x53) // New mode set
	{
		VgaIoWriteIx(0x3D4, 0x1D20);
	}
	else
		VgaIoWriteIx(0x3D4, 0x1C20);
	VgaIoWriteIx(0x3D4, 0x4429);
}
//*********************************************************************
#define sub_594() sub_511() // Same  function
void TR8900D_Init(void)		// No working currently !!
{
	uint16_t i;
	// Int10H  ax=0x1201 bl=0x32 Video addressing disable
	// sub_a60 subsystem enable
	if (!sub_594())
		goto loc_1012;
	IoPortOutB(0x3C8, 0x00);
	do
	{
		IoPortOutB(0x3C9, 0x00);
		i++;
	} while (i < 768);
	IoPortOutB(0x3C2, 0x23);
	sub_5A6();
loc_1012:
	if (sub_594())
		goto loc_106B;
	IoPortInB(0x3DA);
	IoPortOutB(0x3C0, 0x20);
	IoPortInB(0x3DA);
	VgaIoWriteIx(0x3D4, ((VgaIoReadIx(0x3D4, 0x20) & 0xDF) << 8) + 0x20);
loc_106B:
	IoPortOutB(0x3D8, 0x00);
	VgaIoWriteIx(0x3D4, 0x1023);
	IoPortOutB(0x3C6, 0xff);
}

//*********************************************************************
//*********************************************************************
//*********************************************************************

void sub_28A(uint8_t bh)
{
	VgaIoWriteIx(0x3C4, ((VgaIoReadIx(0x3C4, 0x0F) & 0x7F) << 8) + 0x0F);
	VgaIoReadIx(0x3C4, 0x0C);
	VgaIoWriteIx(0x3C4, (bh << 8) + 0x0C);
}
//*********************************************************************
#define sub_29B() sub_26A()
void sub_50C(void)
{ // call sub_122
	uint8_t bh, al;

	bh = sub_29B() | 0x81;
	sub_28A(bh);
	VgaIoWriteIx(0x3C4, (((VgaIoReadIx(0x3C4, 0x0E) & 0x7F) ^ 2) << 8) + 0x0E); // sub_2B2
	if (!(VgaIoReadIx(0x3C4, 0x0F) & 0x08))
		goto loc_187;
	bh = 0;
loc_187:
	VgaIoWriteIx(0x3C4, 0x200D);
	VgaIoWriteIx(0x3C4, 0xA00E);
	VgaIoReadIx(0x3C4, 0x0B);
	VgaIoWriteIx(0x3C4, 0x020E);
	if (!(al = VgaIoReadIx(0x3CE, 0x06)) & 0x0C)
	{
		al &= 0xF3 | 0x04;
		VgaIoWriteIx(0x3CE, (al << 8) + 0x0C);
	}
	VgaIoWriteIx(0x3C4, 0x000D);
	VgaIoWriteIx(0x3D4, 0x001E);
	VgaIoWriteIx(0x3B4, 0x001E);
}
//*********************************************************************
#define sub_4FB() sub_511() // Same  function
void TR9000B_Init(void)
{
	if (!sub_4FB())
		goto loc_1124;
	sub_50C();
	IoPortOutB(0x3C2, 0x23);

loc_1124:
{}
}

void setup()
{
	Serial.begin(1000000);
	Serial.println("HW init.");

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
	digitalWrite(PIN_ALE, LOW); 
	digitalWrite(PIN_AEN, LOW);

	digitalWrite(PIN_IOCHRDY, HIGH);  // Pull-UP

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
	Serial.println("HW init done.");

	IoPortOutB(0x03C3, 0x00);
	IoPortOutB(0x46E8, 0x16);
	IoPortOutB(0x46E9, 0x00);
	IoPortOutB(0x0102, 0x01);
	IoPortOutB(0x0103, 0x00);
	IoPortOutB(0x46E8, 0x0E);
	IoPortOutB(0x46E9, 0x00);
	IoPortOutB(0x4AE8, 0x00);
	IoPortOutB(0x4AE9, 0x00);
	delay(1000);

	uint8_t value, old, chp;

	VgaIoWriteIx(0x3C4, 0x000B); //  Force old_mode_registers
	chp = IoPortInB(0x3C5);		 //  Read chip ID and switch to new_mode_registers}
	old = VgaIoReadIx(0x3C4, 0x0E);
	IoPortOutB(0x3C5, 0x00);
	value = IoPortInB(0x3C5) & 0x0F;
	IoPortOutB(0x3C5, old);

	Serial.print("value: ");
	Serial.print(value, HEX);
	Serial.println();
	if (value == 2)
	{
		IoPortOutB(0x3C5, old ^ 2);
		Serial.print("chp: ");
		Serial.print(chp, HEX);
		Serial.println();
	}

	TR9000i_Init();


	Serial.println("TR9000i init done.");
}

char textBuf[32];
void loop()
{
	/*// red
	ioWrite(0x3C8, 0x0); // index 0
	ioWrite(0x3C9, 0x3F);
	ioWrite(0x3C9, 0x0);
	ioWrite(0x3C9, 0x0);

	delay(1000);

	// green
	ioWrite(0x3C8, 0x0); // index 0
	ioWrite(0x3C9, 0x0);
	ioWrite(0x3C9, 0x3F);
	ioWrite(0x3C9, 0x0);

	delay(1000);

	Serial.println("IO Space:");
	for (uint32_t i = 0x0300UL; i < 0x0400UL; i++)
	{
		if (i % 16 == 0)
		{
			Serial.println();

			snprintf(textBuf, 31, "%08lx", i);
			Serial.print("Address: 0x");
			Serial.print(textBuf);
			Serial.print(": ");
		}

		snprintf(textBuf, 31, "%02x", ioRead(i));
		Serial.print(textBuf);
	}
	
	*/
		
		Serial.println();

	
	while (1)
	{
		Serial.println("Memory Space from 0xA0000:");
		for (uint32_t i = 0xC0000UL; i < 0xD0000UL; i++)
		{
			if (i % 16 == 0)
			{
				Serial.println();

				snprintf(textBuf, 31, "%08lx", i);
				Serial.print("Address: 0x");
				Serial.print(textBuf);
				Serial.print(": ");
			}

			snprintf(textBuf, 31, "%02x", memRead(i));
			Serial.print(textBuf);
		}
	}
}