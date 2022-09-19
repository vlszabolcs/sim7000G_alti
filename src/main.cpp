#include <Arduino.h>

#include <secret.h>
#include <gsm_function.h>
#include <sensor_function.h>
#include <logging.h>

int BP = 0;
const int period_bat=1000;
float v_bat = 0;
const int VReads = 15;

const int period= 1000;
unsigned long time_now = 0;
int function=1;
int last_function;

#include <screen.h>
#include <mqtt.h>


#define PIN_BAT             35




float map_batt(float x, float in_min, float in_max, float out_min, float out_max) {
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
  BP = map_batt(v_bat,2.5,4.2,0,100);     // get battery voltage as a percentage 0-100%
  if (BP < 0) { BP = 0; }
}


void setup(){
  Serial.begin(115200);
  screen_setup();   //Screen setup
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("GSM setup");
  display.display();
  gsm_setup();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("MQTT setup");
  display.display();      //GSM modul start and setup
  mqtt_setup();           //MQTT start and setup
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("SD card setup");
  display.display();     
  sd_card_setup();  //SD card setup 
 
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("BME setup");
  display.display();
  env_sensor.begin(0x76); //Start sensor , default setup
  
  pinMode(LED_PIN, OUTPUT); //Set built in LED
  digitalWrite(LED_PIN, HIGH);
  pinMode(PIN_BAT, INPUT);
}


void loop(){

 if (Serial.available()){     //Check serialMon for function changing
    last_function = function;
    String c;
    c = Serial.readString();
    function=c.toInt();
    };
    
   if (modem.isGprsConnected()) {  mqtt_loop();} //MQTT if gprs connected
  
  switch(function) {
    case 1:                                       // Log mode (default)

    if (!gps_pwr_status){
        Serial.println("GPS power status: "+String(enable_gps(0))); //if GPS is off , turn on 
    }

    if(millis() > time_now + period){ //Wait period minute
        time_now = millis();
        read_bat();
        String log_message=gps_logging()+","+bme280_data()+","+BP; //creat message for logging 
        if (gps_fix()){                                            // if gps fixed start logging 
          logging_csv("GNSS",log_message);                         // log
          dsp_logging();                                           // to screen

        }else{
          dsp_gnss_connecting();                                    // to screen 
          Serial.println("don't logging");
          Serial.println(log_message);  
        }
    }
      break;
    case 2:                             //station mode
      if(millis() > time_now + period){
        time_now = millis();
        just_bme280();                  //sensor update
        dsp_station_mode();             // to screen
      }
      break;

    case 3:
      
      if(millis() > time_now + period_bat){
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
    case 4:   //Debug mode for GSM moduel (AT)

        while(1){
          while (SerialAT.available()) {
            SerialMon.write(SerialAT.read());
          }
          while (SerialMon.available()) {
            SerialAT.write(SerialMon.read());
          }
        }
  
    default:
      if(millis() > time_now + period){
        time_now = millis();

      Serial.println("choos on option 1 or 2");
      }
      break;    
    }
  
  }
   
