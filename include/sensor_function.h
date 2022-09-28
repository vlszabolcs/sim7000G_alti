#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

Adafruit_BME280 env_sensor;

float pres,temp,presAlti,pres_corrig,humi;
String sensor_message;

String bme280_data () { //get sensor data
  pres = env_sensor.readPressure() / 100.0F;
  temp = env_sensor.readTemperature()-1;
  presAlti = env_sensor.readAltitude(pres_corrig);
  humi = env_sensor.readHumidity();
  sensor_message= String(pres)  + "," + String(temp) + "," + String(humi) + "," +String(presAlti);
  return sensor_message;
}

void just_bme280(){     //station mode
  if (gps_pwr_status){
    Serial.println("GPS power status: "+String(disable_gps()));
  }

  bme280_data();
  Serial.println("#######################"); //legyenek azonosak az ilyen elvalasztok, egy masik file-ban ===== vol
  Serial.println(network_time());
  Serial.print("Pres:");
  Serial.println(pres);
  Serial.print("Temp:");
  Serial.println(temp);
  Serial.print("Humi:");
  Serial.println(humi);
  Serial.println("#######################");
  Serial.println();
}
