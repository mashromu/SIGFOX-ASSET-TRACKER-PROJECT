//LETS RECEIVE DATA THAT IS GPGLL FROM THE GPS MODULE
//
#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <TinyGPS.h>

#define GPS_pin 2//this is for waking up the gps module by setting it to high
#define GPS_Info_buffer_size 128
#define waiting_time 15

bool debug=false;// to display values on the serial monitor set the debug to true 

TinyGPS gps;//create an object for the TinyGPS class
            //gps is the object
  //now to set the variable that we ll use to store the data parsed by tinygps class
  //these variables are
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  char GPS_info_char;
  
  char GPS_info_buffer[GPS_Info_buffer_size];
  //stores gps data first 130 characters
  unsigned int received_char;
  bool message_started=false;
  int i=0;

  // create a structure to store and packeage the gps data so that it can be sent
  struct gpscoordinate{
    //will hold lattitude longi and altitude
   float a_latitude;
   float a_longitude;
   float a_altitude;   
    };
    //add an object to access the struct class
    //like gpscoordinates coordinate;

    //to declare the latitude
    float latitude=0.0f;//0.0f states that the 0.0 value should be treated only aas a float
    float longitude=0.0f;
    float altitude=0.0f;


    //this functions introduces delays while blinking the led
void Wait(int m, bool s) {
  //m minutes to wait
  //s slow led pulses
  //debug enables(true) or disables the serial monitor(false)thus the use in the if functions
  
  if (debug) {
    Serial.print("Waiting: "); Serial.print(m); Serial.println(" min.");
  }

  digitalWrite(LED_BUILTIN, LOW);
//if s==true then
  if (s) {

    int seg = m * 30;
    for (int i = 0; i < seg; i++) {
      digitalWrite(LED_BUILTIN, HIGH); //LED on
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW); //LED off
      delay(1000);
    }

  } else {//if s == false then
    //do seg repeats the number of times 15*m
    //introducing a delay of 4000sec *15*m
    int seg = m * 15;
    for (int i = 0; i < seg; i++) {
      digitalWrite(LED_BUILTIN, HIGH); //LED on
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW); //LED off
      delay(3000);

    }
  }
}

//function to convert the received GPS DATA to bytes
String ConvertGPSdata(const void* data, uint8_t len) {
  uint8_t* bytes = (uint8_t*)data;
  String cadena ;
  if (debug) {
    Serial.print("Length: "); Serial.println(len);
  }

  for (uint8_t i = len - 1; i < len; --i) {
    if (bytes[i] < 12) {
      cadena.concat(byte(0));
    }
    cadena.concat(char(bytes[i]));
    if (debug) Serial.print(bytes[i], HEX);
  }

  if (debug) {
    Serial.println("");
    Serial.print("String to send: "); 
    Serial.println(cadena);
  }

  return cadena;
}


//now to get the position data from gpd module
// the function

String GetGPSposition(){
  int message_count=0;
  String pos;

  if(debug)Serial.println("GPS ON");
  digitalWrite(GPS_pin, HIGH);//TURNS ON THE GPS
  //this pin should be used with a transistor since it is very low current and the gps pin requires a large amount of power and current
  Wait(1, false);
  while(message_count<5000){
    while(Serial1.available()){
      int GPS_info_char=Serial1.read();
      if (GPS_info_char=='$') message_count++;
      
      if(debug){
         if (GPS_info_char == '$') { // start of message
          message_started = true;
          received_char = 0;
         }
         else if (GPS_info_char == '*') { // end of message
          for (i = 0; i < received_char; i++) {
            Serial.write(GPS_info_buffer[i]); // writes the message to the PC once it has been completely received
          }
        Serial.println();
          message_started = false; // ready for the new message
         }
  else if (message_started == true) { // the message is already started and I got a new character
          if (received_char <= GPS_Info_buffer_size) { // to avoid buffer overflow
            GPS_info_buffer[received_char] = GPS_info_char;
            received_char++;
          } else { // resets everything (overflow happened)
            message_started = false;
            received_char = 0;
          }

          
          }
      }
    
      if (gps.encode(GPS_info_char)){
        gps.f_get_position(&latitude, &longitude);
        altitude=gps.altitude()/100;
//copy the data to the structure created
        gpscoordinate coordinates={altitude, longitude,latitude};
        gps.crack_datetime(&year, &month, &day,&hour,&minute,&second, &hundredths);
        
        if (debug) {
        Serial.println();
          Serial.println();
          Serial.print("Latitud/Longitud: ");
          Serial.print(latitude, 5);
          Serial.print(", ");
          Serial.println(longitude, 5);
          Serial.println();
          Serial.print("DATE: "); Serial.print(day, DEC); Serial.print("/");
          Serial.print(month, DEC); Serial.print("/"); Serial.print(year);
          Serial.print(" TIME: "); Serial.print(hour, DEC); Serial.print(":");
          Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC);
          Serial.print("."); Serial.println(hundredths, DEC);
          Serial.print("Altitude (METERS): "); Serial.println(gps.f_altitude());
          Serial.print("Direcion(gradient): "); Serial.println(gps.f_course());
          Serial.print("Velocity(kmph): "); Serial.println(gps.f_speed_kmph());
          Serial.print("Satelites: "); Serial.println(gps.satellites());
          Serial.println();
        }

        gps.stats(&chars, &sentences, &failed_checksum);
        if (debug) Serial.println("GPS turned off");
        digitalWrite(GPS_pin, LOW); //GPS turned off
        pos = ConvertGPSdata(&coordinates, sizeof(gpscoordinate)); //Send data
        return pos;
        }
    }
  
  }
  pos="NO Signal";/////////
  }
//now to set up the MKR1200 TO ALLOW FOR TRANSMITTING AND RECEIVING
//FROM AND TO THE SERVER

void SendSigfox(String data)//heres where pos data will be fed to be sent to the server
{
  if (debug){//if serial monitor use is allowed or in place
    Serial.print("sending...");
    Serial.println(data);
    if (data.length()>12){
      Serial.println("message is too long, only 12 bytes will be sent, the rest discarded");
      
      }
    }
  //now to transmit the data we start the mkr 1200
   SigFox.begin();
   //wait for 30 to 100ms for the device to setup
  delay(100);
  //clear all pending interrupts
  SigFox.status();
  delay(1);
  if (debug)SigFox.debug();
  delay(100);

  SigFox.beginPacket();
  SigFox.print(data);

  if(debug){//send buffer to sigfox network and wait for a reply
    int ret=SigFox.endPacket(true);
    if (ret > 0) {
      Serial.println("No transmission");
    } else {
      Serial.println("Transmission ok");
    }

    Serial.println(SigFox.status(SIGFOX));
    Serial.println(SigFox.status(ATMEL));

     if (SigFox.parsePacket()) {
      Serial.println("Response from server:");
      while (SigFox.available()) {
        Serial.print("0x");
        Serial.println(SigFox.read(), HEX);
      }//set a message sent successfull flag high
    }
    else {
      Serial.println("Could not get any response from the server");
      Serial.println("Check the SigFox coverage in your area");
      Serial.println("If you are indoor, check the 20dB coverage or move near a window");
    }//debuging messages
    Serial.println();
  } else {
    SigFox.endPacket();
  }
  SigFox.end();

    
    }
  
  //done with all module set up and functionality code


 //normal coding here
 
void setup() {
  // put your setup code here, to run once:
if (debug) {
    Serial.begin(9600);
    while (!Serial) {}// wait for serial port to connect. Needed for native USB port only
    Serial.println("Serial Connected");
  }
  Serial1.begin(9600);
  while (!Serial1) {}
  if (debug) {
    Serial.println("GPS Connected");
  }
pinMode(GPS_pin, OUTPUT);//connection to the GPS 

if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }

    if (debug) {
    SigFox.debug();
  } else {
    SigFox.end(); // Send the module to the deepest sleep
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
String position_data;

  position_data = GetGPSposition();
   
  SendSigfox(position_data);

  Wait(waiting_time, false);
  
}
