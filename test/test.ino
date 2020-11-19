#include <Adafruit_LEDBackpack.h>
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
  alpha4.begin(0x70);  // pass in the address
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(0, 0x3FFF);
  alpha4.writeDigitAscii(0, 'A');
  alpha4.writeDisplay();
}
