#include "oled-display.h"

////////////////// OLED SETUP //////////////////////////////

void oledSetup() {
    
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        // for (;;); // Don't proceed, loop forever
    }

    display.display();

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    // display.setFont(&FreeMonoBoldOblique12pt7b);
    display.setTextSize(1);
    // display.startscrolldiagleft(0x00, 0xFF);
}


////////////////// OLED DISPLAY //////////////////////////////

void displayParameter(String encoderName, int value, boolean confirmWithClick) {
    if (!client.available()) {
        return;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    // display.setFont(&FreeMonoBoldOblique12pt7b);
    display.setTextSize(2);
    display.dim(1);
    drawCentreString(encoderName, 0);

    if(confirmWithClick) {
        display.setTextSize(4);
        drawCentreString(String(value), 22);
        display.setTextSize(1);
        drawCentreString("Click to confirm", 56);
    } else {
        display.setTextSize(4);
        drawCentreString(String(value), 28);
    }
    display.display();

    Serial.print(encoderName);
    Serial.print(": ");
    Serial.println(value);
}

void drawCentreString(const String &buf, int y)
{
    int16_t x1, y1, x;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
    // display.setCursor(x - w / 2, y);
    display.setCursor((SCREEN_WIDTH - w) / 2, y);

    display.println(buf);
}

void testdrawchar(void) {
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}

void drawQrCode(const char* qrStr, const char* lines[]) {
	uint8_t qrcodeData[qrcode_getBufferSize(3)];
	qrcode_initText(&qrCode, qrcodeData, 3, ECC_LOW, qrStr);
 
  display.clearDisplay();
  display.dim(0);
  Serial.println("Updating OLED display");

  // Text starting point
  int cursor_start_y = 10;
  int cursor_start_x = 4;
  int font_height = 12;

	// QR Code Starting Point
  int offset_x = 62;
  int offset_y = 3;

  for (int y = 0; y < qrCode.size; y++) {
      for (int x = 0; x < qrCode.size; x++) {
          int newX = offset_x + (x * 2);
          int newY = offset_y + (y * 2);

          if (qrcode_getModule(&qrCode, x, y)) {
							display.fillRect( newX, newY, 2, 2, 0);
          }
          else {
							display.fillRect( newX, newY, 2, 2, 1);
          }
      }
  }
  display.setTextColor(1,0);
  for (int i = 0; i < 4; i++) {
    display.setCursor(cursor_start_x,cursor_start_y+font_height*i);
    display.println(lines[i]);
  }
  display.display();
}


void printToSerialAndDisplay(String line1, String line2) {
    // Print to Serial
    Serial.println(line1);
    Serial.println(line1);

  // Print to display
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(line1);
  display.println();
  display.display();
}