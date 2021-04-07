#include <TinyGPS++.h>
#include <ArduinoLowPower.h>
#include<SigFox.h>


//prototypes
void getgpscoordinates();
void sendsigfoxdata();
   

TinyGPSPlus gps;

//structure to packet gps data
struct gpscoordinate{
    //will hold lattitude longitude and altitude
   float a_latitude;
   float a_longitude;
   float a_altitude;   
    }coordinate;

 
    
    float latitudes=0.0f;//0.0f states that the 0.0 value should be treated only as a float
    float longitudes=0.0f;
    float altitudes=0.0f;
    
    bool valid_flag_1=0;
    bool valid_flag_2=0;

    unsigned long start_time;
    unsigned long query_time =20000;// this defines the time the program will take to run through the
                                     //the while serial loop trying to get a valid position
                                   //default is 20 seconds(20000ms)

    /* char lat_str[12];
     char lng_str[12];
     char alt_str[10];*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  my_delay(1000);
  
}

void loop() {
  // put your main code here, to run repeatedly:
//start the gps module


Serial.println("retriving GPS data");

 getgpscoordinates();
  sendsigfoxdata();
   
 

}
void my_delay(unsigned long mseconds ){
  unsigned long current_time= millis();
  while(millis() - current_time < mseconds);
 
  }
void getgpscoordinates(){
 
    start_time=millis();

do{
while(Serial1.available()){
   gps.encode( Serial1.read());//continously feed the encode function with a stream of data from the gps module
   
   if (gps.location.isUpdated()){//checks if the location information has been up dated
    if(gps.location.isValid()){//checks the validity of the updated data.
    latitudes= (gps.location.lat(),6);//retrive and store latitude information
    Serial.print("lat "); Serial.println(latitudes);
    longitudes= (gps.location.lng(),6);//retrive and store longitude information
    Serial.print("long "); Serial.println(longitudes);
    bool valid_flag_1=1;
    }
     else{//if the data is not valid then the everything is fed a zero.
      latitudes=0.0;
      longitudes=0.0;
      Serial.println("invalid location data received, please move to a more open location");
      bool valid_flag_1=0;//valid flag remain zero.
      }
    }
    else if(gps.altitude.isUpdated()){//check if the altitude info is updated
      if(gps.altitude.isValid()){//checks for validity of the received altitude information
        altitudes= (gps.altitude.kilometers(),6);//retrive and save altitude information
        Serial.print("altitude(km) "); Serial.println(altitudes);//print on the serial monitor
        bool valid_flag_2=1;//set valid flag to true
        }
          else{//if the data received is invalid.
            altitudes=0.0;
      Serial.println("invalid altitude data received, please move to a more open location");
            bool valid_flag_2=0;
            
            }
      }
  }
 if((valid_flag_2=1) && (valid_flag_1=1)){break;} //checks if the data received is valid an therefore breaks 
 //from the do while loop. this ensures once valid data is received and stored, the controller exits to perform other tasks
}while(millis()- start_time<query_time);                                 
//do while loop queries the gps module for a redefined period. if no valid data is found within this period then it breaks from the loop

if((millis()- start_time<query_time)<=0){int error=1;}//sets error flag high if the data is not retrived within the predefined period
//function to handle the errors to be added here//

//.........................................//

  if ((valid_flag_2==1) && (valid_flag_1==1)){
     coordinate.a_latitude=latitudes;
     coordinate.a_longitude=latitudes;
    coordinate.a_altitude=altitudes;
    
    }
    else if(valid_flag_1==1 && valid_flag_2==0){
      coordinate.a_latitude=latitudes;
     coordinate.a_longitude=latitudes;
     coordinate.a_altitude=0.0;
      }
      else if(valid_flag_2==1 && valid_flag_1==0){
         coordinate.a_altitude=altitudes;
          coordinate.a_latitude=0.0;
     coordinate.a_longitude=0.0;
        }
 else{
  coordinate.a_latitude=0.0;
     coordinate.a_longitude=0.0;
    coordinate.a_altitude=0.0;
  Serial.println("no gps data found, please change location or check GPS Module connection");
  
  }
  }

  //to send the data to sigfox cloud

  void sendsigfoxdata(){
    //start sigfox
    SigFox.begin();
   //wait for 30 to 100ms for the device to setup
  my_delay(100);
  //clear all pending interrupts
  SigFox.status();
  my_delay(1);
  SigFox.debug();
  my_delay(100);
  
 SigFox.beginPacket();
 SigFox.write((uint8_t*)&coordinate,12);
 //send data bit by bit to sigfox cloud
 uint8_t messagetranStatus= SigFox.endPacket();
 Serial.print("message staus"); Serial.println(messagetranStatus);
 if (messagetranStatus > 0) {
      Serial.println("Data not Transmitted");
    } else {
      Serial.println("Transmission is ok");
    }
 
  Serial.println(SigFox.status(SIGFOX));
    Serial.println(SigFox.status(ATMEL));

     if (SigFox.parsePacket()) {
      Serial.println("Response from server:");
      while (SigFox.available()) {
        Serial.print("0x");
        Serial.println(SigFox.read(), HEX);
      }
     }
     else{
      Serial.println("Could not get any response from the server");
      Serial.println("Check the SigFox coverage in your area");
      Serial.println("If you are indoor, check the 20dB coverage or move near a window");
      Serial.println("configure server backend to send data");
      }
 SigFox.end();
   
  }
