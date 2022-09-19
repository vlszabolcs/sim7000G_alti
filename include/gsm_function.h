#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1
#define SerialMon Serial

#define LED_PIN             12

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]  = "internet";     //SET TO YOUR APN
const char gprs_user[] = "";
const char gprs_pass[] = "";


#include <TinyGsmClient.h>
#include <Ticker.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
#endif

#define uS_TO_S_FACTOR      1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP       60          // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD           9600
#define PIN_DTR             25
#define PIN_TX              27
#define PIN_RX              26
#define PWR_PIN             4

String gps_message;
bool gps_pwr_status;
float lat,lon,spd,gpsAlti;


//BASIC MODULE CONTROLL
void modemPowerOn() {
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1000);    //Datasheet Ton mintues = 1S
    digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff() {
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1500);    //Datasheet Ton mintues = 1.2S
    digitalWrite(PWR_PIN, HIGH);
}

void modemRestart() {
    modemPowerOff();
    delay(1000);
    modemPowerOn();
}

void connect_GPRS(){
    
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprs_user, gprs_pass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");
    if (modem.isGprsConnected()) { 
        SerialMon.println("GPRS connected");
        //modem.NTPServerSync("0.hu.pool.ntp.org", 8);

    }
}

void connect_network(){
    SerialMon.print("Waiting for network...");

    bool is_connected = false;
    int try_count = 60;
    modem.setNetworkMode(38);
    while (try_count--) {
        int16_t signal =  modem.getSignalQuality();
        Serial.print("Signal: ");
        Serial.print(signal);
        Serial.print(" ");
        Serial.print("is_network_connected: ");
        is_connected = modem.isNetworkConnected();
        Serial.println( is_connected ? "CONNECT" : "NO CONNECT");
        if (is_connected) {
            break;
        }
        delay(1000);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }

       delay(1000);
}


void gsm_setup(){               //setup gsm moudle
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(10);
    digitalWrite(PWR_PIN, LOW);             //power on gsm moule (timing form hardwar design manual)
    delay(1010); //Ton 1sec
    digitalWrite(PWR_PIN, HIGH);
    delay(4510);
  
    SerialAT.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX); // Set baud rate
    delay(6000);
        if (!modem.testAT()){
            Serial.println("Failed to restart modem, attempting to continue without restarting");
            //modemRestart();
            return;
        }
    connect_network();      // Connecting LTE network 
    connect_GPRS();         // Connecting to internet 
}

//GPS CONTROL

bool enable_gps(bool autoReport) {      //gps power on
    modem.sendAT("+SGPIO=0,4,1,1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+SGPIO=0,4,1,1 false ");
    }
    
    modem.sendAT("+CGNSPWR=1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CGNSPWR=1 false");
    }   
    
     if (autoReport){
        modem.sendAT("+CGNSURC=1");
        if (modem.waitResponse(10000L) != 1) {
        DBG("+CGNSURC=1 false ");
    }
    }

    modem.sendAT("+CGTP=1");
     if (modem.waitResponse(10000L) != 1) {
        DBG("+CGTP=1 false");
    }

      modem.sendAT("+CGTP?");
     if (modem.waitResponse(10000L) != 1) {
        DBG("+CGTP? false");
    }
    
      modem.sendAT("+CGTP");
     if (modem.waitResponse(10000L) != 1) {
        DBG("+CGTP false");
    }
    
    return gps_pwr_status=true;
}
 
bool disable_gps(void) {            // gps power off
  
    modem.sendAT("+SGPIO=0,4,1,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+SGPIO=0,4,1,0 false ");
    }
     modem.sendAT("+CGNSPWR=0");
     if (modem.waitResponse(10000L) != 1) {
        DBG("+CGNSPWR=0 false");
    }
    return gps_pwr_status=false;
}

String gps_logging(){   //get gps raw csv data

     return modem.getGPSraw();

   /*if (SerialAT.available()) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            //c=SerialAT.read();  
           
            Serial.println(gps_message);
            //strtok(String(c),",");
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            
        }*/
}

bool gps_fix(){ //get parsed gps data for 3D fixing 
   
    if(!modem.getGPS(&lat,&lon,&spd,&gpsAlti)){
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(10);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }

    return modem.getGPS(&lat,&lon,&spd,&gpsAlti); 
}

String network_time(){  // get networ time
    int   year,month,day,hour,min,sec;
    float timezone;
   
    modem.getNetworkTime(&year,&month,&day,&hour,&min,&sec,&timezone);
    String networkTime=String(year)+String(month)+String(day)+String(hour)+String(min)+String(sec);
    
    return networkTime;
}