#include <PubSubClient.h>
PubSubClient  mqtt(client);



#define MQTT_BROKER   "io.adafruit.com"
#define MQTT_PORT     1883
#define MQTT_USER     "AxlHacke"
#define MQTT_PASSWORD "aio_vXun76Y1l5DpdtDpgqLhh7FgM8QC"

const char* pres_topic="AxlHacke/feeds/bme280.pressure";
const char* temp_topic="AxlHacke/feeds/bme280.temperature";
const char* humi_topic="AxlHacke/feeds/bme280.humidity";

uint32_t lastReconnectAttempt = 0;

const int periodMQTT= 60000;
unsigned long time_nowMQTT = 0;

bool status=0;

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}


boolean mqttConnect() {
  Serial.print("Connecting to ");
  Serial.print(MQTT_BROKER);

  // Connect to MQTT Broker
  boolean status = mqtt.connect("SIM7000G_alti",MQTT_USER,MQTT_PASSWORD);

  // Or, if you want to authenticate MQTT:
  // boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    Serial.println(" fail");
    return false;
  }
  Serial.println(" success");
  
  mqtt.subscribe("AxlHacke/feeds/photocell");
  return mqtt.connected();
}

bool mqttSetup(){
    connectGPRS();
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
    status = true;
   return status;
}



void mqttPublish ( const char* topicsPath,float data){
    char conString[8];
    dtostrf(data , 1, 2, conString);
    Serial.print(topicsPath);
    Serial.println(conString);
    mqtt.publish(topicsPath, conString );
}

void mqttLoop(){
  
    

   
   if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) { lastReconnectAttempt = 0; }
    }
    delay(100);
    return;
  }

   if(millis() > time_nowMQTT + periodMQTT){
    time_nowMQTT = millis();
     
    
   

        mqttPublish(pres_topic,pres);
        mqttPublish(temp_topic,temp);
        mqttPublish(humi_topic,humi);
    }
}


