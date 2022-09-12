String head = "GNSS status,Fix status,dTime,Latitude,Longitude,MSL Altitude,Speed ,Course,Fix Mode,Reserved1,HDOP,PDOP,VDOP,Reserved2,GNSS Satellites in View,GPS Satellites Used,GLONASS Satellites used,Reserved3,C/N0 max,HPA,VPA,pressure,temperature,humidity,altitude,Battery";

String path;
File file;
bool once_run = false;
bool file_name_exist= false;



bool creat_directory(String directory_name){
    bool success=false;
    if (!SD.exists("/"+directory_name))
    {
        if (SD.mkdir("/"+directory_name)){
            Serial.println(directory_name+" is directory CREATED");
            success = true;
        }else{
            Serial.println("Directory creation is FAILD");
        }
    }
    else {
        Serial.println(directory_name+" is exists!");
        success=true;
    }
    return success;
}

bool creat_file_name(String logging_mode ){

    if(!once_run){
        int   year,month,day,hour,min,sec;
        float timezone;
        
        modem.getNetworkTime(&year,&month,&day,&hour,&min,&sec,&timezone);

        String fileName=String(year)+String(month)+String(day)+"_"+String(hour)+String(min)+String(sec)+"_"+logging_mode;
        String folderName=String(year)+String(month)+String(day);

        Serial.println(fileName);

            if (creat_directory(folderName)){

                path ="/"+folderName+"/" + fileName + ".csv";
                
                Serial.println(path);

                file = SD.open(path, FILE_WRITE);
                
                if (file) {
                    file.println(head);
                } else {
                    Serial.println("Failed to open file for writing");}

            once_run = true;
            
            }
    } 
    return once_run;
}

void logging_csv(String log_mode, String message){
    
    if(creat_file_name(log_mode)){
        if(file) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            file.println(message);
            file.flush();
            Serial.println(message);
            Serial.println("Logging run!");
        }else{Serial.println("Logging file faild to open!");}    
    }else{
        Serial.println("don't logging");
        Serial.println(message);
        
        
    }
}


