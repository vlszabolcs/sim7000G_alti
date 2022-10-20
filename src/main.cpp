#include <Arduino.h>

#include <secret.h>
#include <gsm_function.h>
#include <sensor_function.h>
#include <logging.h>

// +-*/= es == operatorok altalaban szokozzel vannak korbeveve
// figyelj a behuzasra legyen azonos minden file-ben pl 2 szokoz! nem tab!
int BP = 0;
const int period_bat = 1000;
float v_bat = 0;
const int VReads = 15;

const int period = 1000;
unsigned long time_now = 0;
const int station_period = 60000;
unsigned long station_time_now = 0;
int function = 1;
int last_function;

#include <screen.h>
#include <mqtt.h>


#define PIN_BAT 35



// valassz ki egy fgv strukturat es az kovesd mindenhol:
// float map_batt(blabla)
// {
// ...
// }
// vagy
// float map_batt(blabla){
// ...
// }
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
    //sortData(voltageBuffer, VReads);  //az ilyen dead code-ot vedd ki
    Read_buffer += (voltageBuffer[(VReads - 1) / 2]);
  }
  v_bat = (((float)(Read_buffer / VReads) / 4096) * 3600 * 2) / 1000;
  BP = map_batt(v_bat,2.5,4.2,0,100);     // get battery voltage as a percentage 0-100% //dead code
  if (BP < 0) { BP = 0; } // ez nem tul szep
}


void setup() {
  Serial.begin(115200);
  screen_setup(); //Screen setup
  gsm_setup(); //GSM modul start and setup
  mqtt_setup(); //MQTT start and setup
  sd_card_setup();  //SD card setup
  env_sensor.begin(0x76); //Start sensor , default setup
  pinMode(LED_PIN, OUTPUT); //Set built in LED
  digitalWrite(LED_PIN, HIGH);
  pinMode(PIN_BAT, INPUT);
}


void loop(){

 if (Serial.available()) {     //Check serialMon for function changing
    last_function = function;
    String c;
    c = Serial.readString();
    function=c.toInt();
    };

   if (modem.isGprsConnected()) {  mqtt_loop();} //MQTT if gprs connected

  switch(function) {
    case 1:                                       // Log mode (default)

    if (!gps_pwr_status){
        Serial.println("GPS power status: "+String(enable_gnss(0))); //if GPS is off , turn on
    }

    if(millis() > time_now + period) { //Wait period minute
        time_now = millis();
        read_bat();
        String log_message=gnss_data()+","+bme280_data()+","+BP; //creat message for loging
        if (gnss_fix()) {                                            // if gps fixed start loging
          logging_csv("GNSS",log_message);                         // log
          dsp_loging();  

        }else {
          dsp_gnss_connecting();                                    // to screen
          Serial.println("don't loging");
          Serial.println(log_message);
        }
    }
      break;
    case 2:                             //station mode
      if(millis() > station_time_now + station_period){
        station_time_now = millis();
        String msg= network_time() + ","+bme280_data();
        logging_csv("BME280",msg);                 
      }

      if(millis() > time_now + period){
        time_now = millis();                 //sensor update
        mode_station();
        dsp_station_mode();  // to screen
      }

      break;

    case 3:

      if(millis() > time_now + period_bat) {
        time_now = millis();

        read_bat();
        String bat_message = network_time() + "," + String(v_bat) + "," + String(BP);
        logging_csv("BAT",bat_message);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print((v_bat));
        display.println("V");
        display.print((BP));
        display.println("%");
        display.display();

        Serial.print("Battery Voltage : "); Serial.println(v_bat);  // 2.5v - 4.2v //egy sor egy utasitas
        Serial.print("Battery Percentage : "); Serial.print(BP);  Serial.println("%");  // 0-100%
      }
      break;
    case 4:   //Debug mode for GSM moduel (AT)

        while(1){
          while (SerialAT.available()) {
            Serial.write(SerialAT.read());
          }
          while (Serial.available()) {
            SerialAT.write(Serial.read());
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

