#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
byte ledPort = 13; // digital pin for LED
byte nibbles[10]; // array to hold the nibbles
char encodedString[41]; // allow additional byte for null terminator

float testTemperature = -20.0;
    
void setup() 
{

  Serial.begin(9600);
  pinMode(ledPort, OUTPUT);    
    
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);

  // set protocol one
  mySwitch.setProtocol(1);
  
  // set pulse length.
  mySwitch.setPulseLength(500);
  
  // Optional set number of transmission repetitions.
  mySwitch.setRepeatTransmit(3);
}

void loop() 
{
  // I'm not really sure what the Sensor Address values are supposed to represent
  // Possibly a house code and a channel, although they just resolve to an integer
  
  buildNibbles(1, 2, testTemperature);
  zeroString(encodedString, 40); // set all the bits to zero
  buildPayload(encodedString);

  Serial.print("Sending TX3 temperature : ");
  Serial.println(testTemperature);  
  Serial.print(" Encoded: ");
  Serial.println(encodedString);
  notString(encodedString, 40);
  Serial.print("Inverted: ");
  Serial.println(encodedString);
  rightShiftString(encodedString, 40);
  Serial.print(" Shifted: ");
  Serial.println(encodedString);
  
  mySwitch.send(encodedString);

  testTemperature += 1.3; // just some arbitrary incrementer for testing
  
  if(testTemperature > 50)
   testTemperature = -20.0;
  
  blinkLed();
  delay(5000);
}

void buildNibbles(byte addressA, byte addressB, float temperature)
{
  //nibble 0 : Always 0xA
  //nibble 1 : 0 for temperature
  //nibble 2 & 3 : Sensor address - probably House and Channel codes
  //nibble 4 & 5 : Unused (we can use these to ensure parity nibble is odd)
  //nibble 6 : Encoded temperature units
  //nibble 7 : Encoded temperature hundreds
  //nibble 8 : Encoded temperature tens (note the strange order)
  //nibble 9 : Checksum (sum of all nibbles)
  
  unsigned int convertedTemp = (temperature + 50) * 10;
  unsigned int tempHundreds = convertedTemp / 100;
  unsigned int tempTens = (convertedTemp % 100) / 10;
  unsigned int tempUnits = convertedTemp % 10;
  nibbles[0] = 0xA;
  nibbles[1] = 0x0;
  nibbles[2] = 0x3;
  nibbles[3] = 0x1;
  nibbles[4] = 0x0;
  nibbles[5] = 0x0;
  nibbles[6] = tempUnits & 0xF;
  nibbles[7] = tempHundreds & 0xF;
  nibbles[8] = tempTens & 0xF;
  nibbles[9] = 0x0;
  
  buildParity();
}
  
void buildParity()
{
  // due to a quirk in the way the RF bits are received, the last bit will always be one
  // so we need to ensure that the checksum nibble always ends in one by setting one of the
  // unused nibbles to be one.
  byte  checksum = 0;
  
  for(int i = 0; i < 9; i++)
    checksum += nibbles[i];
  
  if((checksum & 1) != 1)
  {
    Serial.println("parity adjusted");
    checksum += 1;
    nibbles[4] = 1;
  }
  nibbles[9] = checksum & 0xF;
}

void buildPayload(char *s)
{
  for(int i = 0; i < 10; i++)
    insertNibble(&s[i * 4], nibbles[i]);
}

void zeroString(char *s, unsigned int l)
{
  for(int i = 0; i < l; i++)
    s[i] = '0'; 
  s[l] = '\0'; // null terminate string
}

void insertNibble(char *s, byte n)
{
  for(int i = 3; i >= 0; i--)
  {
    s[i] = ((n & 1) > 0) ? '1' : '0';
    n = n >> 1;
  }
}

void notString(char *s, unsigned int l)
{
  for(int i = 0; i < l; i++)
    s[i] = (s[i] == '0') ? '1' : '0';
}

void rightShiftString(char *s, unsigned int l)
{
  for(int i = l-1; i > 0; i--)
    s[i] = s[i-1];
    
  s[0] = '0'; // fill left bit with a zero
}

void blinkLed()
{
  // blink the led
  digitalWrite(ledPort, HIGH);
  delay(30);   
  digitalWrite(ledPort, LOW);
}
