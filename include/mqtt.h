#include <PubSubClient.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;

PubSubClient  mqtt(client);
uint32_t last_reconnect_attempt = 0;

const int peroid_MQTT = 60000;
const int peroid_loc = 15000;
unsigned long time_now_loc = 0;
unsigned long time_now_MQTT = 0;
bool status=0;

void mqtt_callback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  //Serial.write(payload,len);
  char buff_p[len];
  for (int i = 0; i < len; i++){
    Serial.print((char)payload[i]);
    buff_p[i] = (char)payload[i];
  }
  buff_p[len] = '\0';
  String msg_p = String(buff_p);
  float income = msg_p.toFloat(); //to float
  if (String(topic)==String(slp_topic)){
    Serial.print("Ez nyomás: ");
    Serial.print(income);
    pres_corrig=income;
    (SD.open("/lastPresCalc.txt",FILE_WRITE)).print(income);
  }
  else if(String(topic)==String(device_func)){
    Serial.print("Ez a menü: ");
    Serial.print(income);
    function=income;
  }

  Serial.println();
}


boolean mqtt_connect() {
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

  mqtt.subscribe(slp_topic);
  mqtt.subscribe(device_func);

  return mqtt.connected();
}

bool mqtt_setup(){
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(mqtt_callback);
    status = true;
    return status;
}

void float_publish ( const char* topics_path,float data){
    char con_string[8];
    dtostrf(data , 1, 2, con_string);
    Serial.print(topics_path);
    Serial.println(con_string);
    mqtt.publish(topics_path, con_string );
}

void location (){
  String msg;

  /*JsonArray value = doc.createNestedArray("value");
  value.add(String(temp,1));
  value.add(String(humi,0));
  value.add(String(pres,2));*/
  doc["value"] = String(temp,1);
  doc["lat"] = String(lat,4);
  doc["lon"] = String(lon,4);
  doc["ele"] = String(pres_alti,2);
  deserializeJson(doc,msg);
  Serial.println(msg);
  char Buf[150];
  msg.to_char_array(Buf, 150);
  mqtt.publish(gps_topic,Buf);
}



void mqtt_loop(){
  if (!mqtt.connected()) {
    Serial.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - last_reconnect_attempt > 10000L) {
      last_reconnect_attempt = t;
      if (mqtt_connect()) { last_reconnect_attempt = 0; }
    }
    delay(100);
    return;
  }
    mqtt.loop();
  
  if(millis() > time_now_MQTT + peroid_MQTT){
    time_now_MQTT = millis();
    mqtt.publish(sim7000g_status,"1");
    float_publish(alti_topic,pres_alti);
    float_publish(pres_topic,pres);
  }

  if(millis() > time_now_loc + peroid_loc){
    time_now_loc= millis();
    if (gnss_fix()){
      location();  
    }    
  } 
}


