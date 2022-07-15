#include <Adafruit_BME280.h>
#include <Wire.h>

Adafruit_BME280 envSensor;

float pres,temp,presAlti,presCorrig,humi;
String sensor_message;



String bme280_data () {
  
  pres = envSensor.readPressure() / 100.0F;
  temp = envSensor.readTemperature()-6;
  presAlti = envSensor.readAltitude(presCorrig);
  humi = envSensor.readHumidity();
  sensor_message= String(pres)  + "," + String(temp) + "," + String(humi) + "," +String(presAlti);
  return sensor_message;
}

void justBME280(){
  if (gps_pwr_status){
    Serial.println("GPS power status: "+String(disableGPS()));
  }

  bme280_data();
  Serial.println("#######################");
  Serial.println(modem.getGSMDateTime(DATE_FULL));
  Serial.print("Pres:");
  Serial.println(pres);
  Serial.print("Temp:");
  Serial.println(temp);
  Serial.print("Humi:");
  Serial.println(humi);
  Serial.println("#######################");
  Serial.println();
}