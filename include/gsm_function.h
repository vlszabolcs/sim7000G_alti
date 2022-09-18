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
const char gprsUser[] = "";
const char gprsPass[] = "";

const char wifiSSID[] = "ASUS_28_2G";
const char wifiPass[] = "28e6e613";



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

void connectGPRS(){
    // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");
    if (modem.isGprsConnected()) { 
        SerialMon.println("GPRS connected"); 

    }
  
    /*
    modem.sendAT("+CNTPCID=1");   
    delay(10);
    String server="1.hu.pool.ntp.org";
    modem.sendAT("+CNTP="+server+",8");
    delay(5000);
    SerialAT.write("AT+CNTP");
    */
    //modem.sendAT("+CNTP");
    //modem.NTPServerSync("0.hu.pool.ntp.org", 8);
    //delay(1000);
    //SerialAT.write("AT+CNTP");
}

void connectNetwork(){

    SerialMon.print("Waiting for network...");

    bool isConnected = false;
    int tryCount = 60;
    modem.setNetworkMode(38);
    while (tryCount--) {
        int16_t signal =  modem.getSignalQuality();
        Serial.print("Signal: ");
        Serial.print(signal);
        Serial.print(" ");
        Serial.print("isNetworkConnected: ");
        isConnected = modem.isNetworkConnected();
        Serial.println( isConnected ? "CONNECT" : "NO CONNECT");
        if (isConnected) {
            break;
        }
        delay(1000);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }

       delay(1000);
}


void gsmSetup(){
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(10);
    digitalWrite(PWR_PIN, LOW);
    delay(1010); //Ton 1sec
    digitalWrite(PWR_PIN, HIGH);
    delay(4510);
  // Set baud rate
  SerialAT.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(6000);
    if (!modem.testAT()){
        Serial.println("Failed to restart modem, attempting to continue without restarting");
        //modemRestart();
        return;
    }
    connectNetwork();
    connectGPRS();
}

//GPS CONTROL

bool enableGPS(bool autoReport) {
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
 
bool disableGPS(void) {
  
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

String gpsLogging(){

     return modem.getGPSraw();

   /*if (SerialAT.available()) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            //c=SerialAT.read();  
           
            Serial.println(gps_message);
            //strtok(String(c),",");
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            
        }*/
}

bool gpsFix(){
   
    if(!modem.getGPS(&lat,&lon,&spd,&gpsAlti)){
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(10);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }

    return modem.getGPS(&lat,&lon,&spd,&gpsAlti); 
}

String network_time(){
    int   year,month,day,hour,min,sec;
    float timezone;
   
    modem.getNetworkTime(&year,&month,&day,&hour,&min,&sec,&timezone);
    String networkTime=String(year)+String(month)+String(day)+String(hour)+String(min)+String(sec);
    
    return networkTime;
}