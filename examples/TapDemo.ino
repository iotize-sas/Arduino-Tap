#include <Tap.h>

unsigned short var1[3] = { 0x8899, 0xAABB, 0xccdd }; 
unsigned short counter = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  Serial.println("start!");
#if (defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)) 
  myTap.Init(3, 5);  //Start S3P with CLK 
#elif defined(ARDUINO_SAM_DUE) 
  myTap.Init(16, 17);  //Start S3P with CLK
#endif
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // prints title with ending line break
  Serial.println("Waiting for S3P irq!");
}

// first visible ASCIIcharacter '!' is number 33:
short int thisByte = 33;
extern char *pdebug;
extern int *pvalue ;

// you can also write ASCII characters in single quotes.
// for example, '!' is the same as 33, so you could also use this:
// int thisByte = '!';

void loop() {
  // prints value unaltered, i.e. the raw binary version of the byte.
  // The Serial Monitor interprets all bytes as ASCII, so 33, the first number,
  // will show up as '!'

  if ( (counter++ % 25) == 0 )
  {
    Serial.print("\nCounter=");
    Serial.print(counter, DEC);
  }
  if (0 && pdebug)
  {
    Serial.print("\nDebug=");
    Serial.print(pdebug);
    Serial.print(" with value= ");
    Serial.print(*pvalue, DEC);
    pdebug = 0;
  }
  delay(100);
  var1[2]++;
}
