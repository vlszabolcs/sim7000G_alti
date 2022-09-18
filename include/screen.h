#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void screen_setup(){
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
    }
    display.display();
    display.clearDisplay();
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
}

void gnss_connecting(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("GNSS connecting...");
    display.println(gps_pwr_status);
    display.println(bme280_data());
    display.print((BP));
    display.println("%");
    display.display();
}

void gnss_conected(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(String(lat)+";"+String(lon)+";"+String(gpsAlti));
    display.println(bme280_data());
    display.print((BP));
    display.println("%");
    display.display();
}

void station_mode(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println((network_time()));
   /* display.print(("Bat"));
    display.print((BP));
    display.println("%");*/
    display.print("Temp: ");
    display.print(temp);
    display.println("*C");
    display.print("Humi: ");
    display.print(humi);
    display.println("%");
    display.print("Pres: ");
    display.print(pres);
    display.println("hPa");
 
    display.display();
}


