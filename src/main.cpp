#include <Arduino.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <SD.h>

#include <gsm_function.h>
#include <bmp280_function.h>
#include <logging.h>
#include <mqtt.h>
#include <screen.h>

const int period= 1000;
const int periodBat=1000;
unsigned long time_now = 0;
int function=1;
int lastFunction;

int BP = 0;
float v_bat = 0;
const int VReads = 15;

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13


#define PIN_BAT             35



void sdCardSetup(){
 //Read data form SD Card
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  File fileRead = SD.open("/lastPresCalc.txt");
  if (fileRead) {
    presCorrig = fileRead.parseFloat();
    Serial.println(presCorrig);
    fileRead.close();
  }
  else {
    Serial.println("Error to open last pressure calc.");
  }
}

float mapBatt(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void read_bat() {         // reads and returns the battery voltage
  float voltageBuffer[VReads];
  uint32_t Read_buffer = 0;
  for (int x = 0; x < VReads; x++) {
    for (int i = 0 ; i < VReads; i++) {
      voltageBuffer[i] = (uint32_t)analogRead(PIN_BAT);
    }
    //sortData(voltageBuffer, VReads);
    Read_buffer += (voltageBuffer[(VReads - 1) / 2]);
  }
  v_bat = (((float)(Read_buffer / VReads) / 4096) * 3600 * 2) / 1000;
  BP = mapBatt(v_bat,2.5,4.2,0,100);     // get battery voltage as a percentage 0-100%
  if (BP < 0) { BP = 0; }
}



void setup(){
  Serial.begin(115200);
  envSensor.begin(0x76);
  screen_setup();
  sdCardSetup();
  
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  pinMode(PIN_BAT, INPUT);

  gsmSetup();
  
  
  Serial.println("EZ A SETUP IDŐ:"+modem.getGSMDateTime(DATE_FULL));
  delay(1000);
  //mqttSetup();
}
void loop(){

  /*if (SerialAT.available()) {
    SerialMon.write(SerialAT.read());
  }
  if (SerialMon.available()) {
    SerialAT.write(SerialMon.read());
  }*/
 if (Serial.available()){
    lastFunction = function;
    String c;
    c = Serial.readString();
    function=c.toInt();
    };
    
  
  
  switch(function) {
    case 1: // Logging
    if (!gps_pwr_status){
        Serial.println("GPS power status: "+String(enableGPS(0)));
    }

    if(millis() > time_now + period){
        time_now = millis();
        
        String gnss_message=gpsLogging()+","+bme280_data();
        if (gpsFix()){
        logging_csv("GNSS",gnss_message);
        }else{
          Serial.println("don't logging");
          Serial.println(gnss_message);
        }
      
               
      }
      //Serial.println(head);
      break;
    case 2: //BME Function
      if(millis() > time_now + period){
        time_now = millis();
        justBME280();
        //mqttLoop();        
      }
      break;

    case 3:
      
      if(millis() > time_now + periodBat){
        time_now = millis();

        read_bat();
        String bat_message = network_time()+","+String(v_bat)+","+String(BP);
        logging_csv("BAT",bat_message);
        
  
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print((v_bat));
        display.println("V");
        display.print((BP));
        display.println("%");
        display.display();

        
        Serial.print("Battery Voltage : "); Serial.println(v_bat);  // 2.5v - 4.2v
        Serial.print("Battery Percentage : "); Serial.print(BP);  Serial.println("%");  // 0-100%
      }
      break;
    case 4:
      Serial.println();
      Serial.println("new function: "+String(function));
      Serial.println("old function: "+String(lastFunction));
      Serial.println();
      break;
    default:
      Serial.println("choos on option 1 or 2");
      break;
      
      
    }
  
  }
   
