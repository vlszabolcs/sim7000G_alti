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



void dsp_gnss_connecting(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("GNSS connecting...");
    display.println(gps_pwr_status);
    display.println(bme280_data());
    display.print((BP));
    display.println("%");
    display.display();
}

float old_pres_alti=0;
float uphill=0;
float downhill=0;

void ele(){
    float del_alti;
    if (old_pres_alti!=0){
        del_alti=pres_alti-old_pres_alti;
    }
    else{
        old_pres_alti=pres_alti;
    }
    
    if (del_alti>0){
        uphill+=del_alti;
        old_pres_alti=pres_alti;
    } 
    else{
        downhill+=del_alti;
        old_pres_alti=pres_alti;  
    }
}

void dsp_loging(){
    ele();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(network_time());
    display.println(String(lat)+";"+String(lon));
    display.print("A:");
    display.print(pres_alti);
    display.print(" G:");
    display.println(gpsAlti);
    display.print((BP));
    display.println("%");
    display.display();
}



void dsp_station_mode(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println((network_time()));
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




