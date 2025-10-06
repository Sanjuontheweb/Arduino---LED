#include "LedControl.h"

// 12 is for data pin
//11 is for CLK pin
// 10 is for CS pin
LedControl matrix = LedControl(12, 11, 10,1 );

String getValue(String data, char seperator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i=0; i < maxIndex && found <= index; i++) {
    if(data.charAt(i) == seperator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup() {
  // put your setup code here, to run once:
  matrix.shutdown(0, false); //wake up the led matrix
  matrix.setIntensity(0, 8); // sets the brightness of the leds
  matrix.clearDisplay(0); //clear everything
  Serial.begin(115200); // baud rate which is the number of zeros and one through the usb per second
  Serial.setTimeout(10); // how long in mulliseconds the aeduino will wair dor a  mesaaga
}

void loop() {
  String message = Serial.readStringUntil('$');
  message.trim();

  if (message.length() == 0) {
    return;
  }

  for (int row = 0; row < 8; row += 1) {
    for (int col = 0; col < 8; col += 1) {
      bool isOn = getValue(message, ":", row * 8 + col) == "1";
      matrix.setled(0, col, row, isOn)
    }
  }
}
