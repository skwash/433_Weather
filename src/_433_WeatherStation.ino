// Oregon V2 decoder added - Dominique Pierre
// Oregon V3 decoder revisited - Dominique Pierre
// New code to decode OOK signals from weather sensors, etc.
// 2010-04-11 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// $Id: ookDecoder.pde 5331 2010-04-17 10:45:17Z jcw $

#include <OregonSci.h>

OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;

#define PORT 2

volatile word pulse;

// Serial activity LED
const int serialLED = 7;
// Sensor activity LED
const int sensorLED = 6;

// Details for my sensor:
const int sensorChannel = 2;
const int sensorCode = 0xEC;

// Serial variables
const int serialLength = 32; // size of the serial buffer
char serialString[serialLength];
byte serialIndex = 0;

char lasttemp[6] = "0.0";
char lasthumid[3] = "0";
long lasttime = 0;

#if defined(__AVR_ATmega1280__)
void ext_int_1(void) {
#else
ISR(ANALOG_COMP_vect) {
#endif
    static word last;
    // determine the pulse length in microseconds, for either polarity
    pulse = micros() - last;
    last += pulse;
}

void parseData (const char* s, class DecodeOOK& decoder) {
    byte pos;
    const byte* data = decoder.getData(pos);
    Serial.println(3133);
    // TODO: something sensible with the sensor code
    // if ((int)(data[2] >> 4) == sensorChannel && (int)data[3] == sensorCode) {
    if ((int)(data[2] >> 4) == sensorChannel) {
      // Get the temperature.
      char temp[6];
      char *tempptr = &temp[0];
      // 14th nibble indicates sign. non-zero for -ve
      if ((int)(data[6] & 0x0F) != 0) {
    *tempptr = '-';
    tempptr++;
      }
      sprintf(tempptr, "%02x", (int)(data[5]));
      tempptr = tempptr + 2;
      *tempptr = '.';
      tempptr++;
      sprintf(tempptr, "%x", (int)(data[4] >> 4));

      // Get the humidity.
      char humid[3];
      char *humidptr = &humid[0];
      sprintf(humidptr, "%x", (int)(data[7] & 0x0F));
      humidptr++;
      sprintf(humidptr, "%x", (int)(data[6] >> 4));
      humid[2] = '\0';

      strcpy(lasttemp, temp);
      strcpy(lasthumid, humid);
      Serial.println(44444);
      lasttime = millis();
    }

    decoder.resetDecoder();
}

void readSerial() {
  
  while ((Serial.available() > 0) && (serialIndex < serialLength-1)) {
    digitalWrite(serialLED, HIGH);
    char serialByte = Serial.read();
    if (serialByte != ';') {
      serialString[serialIndex] = serialByte;
      serialIndex++;
    }
    if (serialByte == ';' or serialIndex == (serialLength-1)) {
      //parseSerial();
      serialIndex = 0;
      memset(&serialString, 0, serialLength);
    }
  }
  digitalWrite(serialLED, LOW);
}

void parseSerial() {
  if (strcmp(serialString, "get") == 0) {
    long age = (millis() - lasttime) / 1000;
    // Format is "channel,temp,humidity,age;"
    Serial.print("1,");
    Serial.print(lasttemp);
    Serial.print(",");
    Serial.print(lasthumid);
    Serial.print(",");
    Serial.print(age);
    Serial.print(";\n");
  } else {
    Serial.print("nope");
  }
}

void werp(){
    Serial.print("werp\n");
    Serial.print("1,");
    Serial.print(lasttemp);
    Serial.print(",");
    Serial.print(lasthumid);
    //Serial.print(",");
    Serial.print(";\n");
}

void reportSerial (const char* s, class DecodeOOK& decoder) {
    byte pos;
    const byte* data = decoder.getData(pos);
    Serial.print(s);
    Serial.print(' ');
    for (byte i = 0; i < pos; ++i) {
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0x0F, HEX);
    }
    
    // Serial.print(' ');
    // Serial.print(millis() / 1000);
    Serial.println();
    
    decoder.resetDecoder();
}


void setup () {
  Serial.begin(115200);
  delay(1000);
  //Serial.println("HELLO");
  pinMode(serialLED, OUTPUT);
  digitalWrite(serialLED, HIGH);
  pinMode(sensorLED, OUTPUT);
  digitalWrite(sensorLED, HIGH);
  delay(1000);
  digitalWrite(serialLED, LOW);
  digitalWrite(sensorLED, LOW);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

#if !defined(__AVR_ATmega1280__)
    pinMode(13 + PORT, INPUT);  // use the AIO pin
    digitalWrite(13 + PORT, 1); // enable pull-up

    // use analog comparator to switch at 1.1V bandgap transition
    ACSR = _BV(ACBG) | _BV(ACI) | _BV(ACIE);

    // set ADC mux to the proper port
    ADCSRA &= ~ bit(ADEN);
    ADCSRB |= bit(ACME);
    ADMUX = PORT - 1;
#else
   attachInterrupt(1, ext_int_1, CHANGE);

   DDRE  &= ~_BV(PE5);
   PORTE &= ~_BV(PE5);
#endif

   // Serial.println("\n[weatherstation initialised]");
}

void loop () {
  static int i = 0;
    int j = 0;
    cli();
    word p = pulse;
    
    pulse = 0;
    sei();

    //if (p != 0){ Serial.print(++i); Serial.print('\n');}
      //Serial.println(p); 
   //delay(1000);   

    /*
    for (int j=0;j<p;j++) {
      digitalWrite(sensorLED, HIGH);
      delay(100);
      digitalWrite(sensorLED, LOW);
      delay(100);      
      Serial.println(p);
    }
    delay(1000);
    */
    
    if (p != 0) {
      digitalWrite(sensorLED, HIGH);
      if (orscV2.nextPulse(p)) {
    digitalWrite(sensorLED, HIGH);
        reportSerial("OSV2", orscV2);
    parseData("OSV2", orscV2);
        werp();


    digitalWrite(sensorLED, LOW);
        //Serial.println("herro");

      }
      digitalWrite(sensorLED, LOW);
    }
    

    readSerial();
    
    //Serial.println("HELLO");
    
}
