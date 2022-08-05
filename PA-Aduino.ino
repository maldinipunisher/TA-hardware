#include<SoftwareSerial.h>
#include <TinyGPSPlus.h>
#define TINY_GSM_MODEM_SIM900
#define TINY_GSM_RX_BUFFER 270
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

const String password = "123456";
void(* reset)(void)=0;
const char server[] = "arduino-fs.herokuapp.com";
const int  port = 80;
String LOCK;
static const uint32_t GPSBaud = 9600;
bool isFirstTime = true;

// The TinyGPSPlus object
TinyGPSPlus gps;
SoftwareSerial gsm(7,8);
TinyGsm modem(gsm);
TinyGsmClient client(modem);
HttpClient http(client, server, port);

void setup()
{
  pinMode(12, OUTPUT); 
  pinMode(13, OUTPUT);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  pinMode(5, INPUT_PULLUP);
  Serial.begin(9600);
  gsm.begin(9600);
  powerSIM(); 
  delay(3000); 
  Serial.println(F("DeviceExample.ino"));
  if(!modem.waitForNetwork())
  {
    reset();
  }
  if (!modem.gprsConnect("internet", "", ""))
  {
    reset();
  }
  Serial.print(F("start modem: "));Serial.println(String(modem.getModemInfo()));
}

void loop()
{
  while (Serial.available() > 0)
    {
      if (gps.encode(Serial.read()))
      {
        if(isFirstTime) 
        {
          http.get("/setup?status=on&lock=off&password=" + password);
          if(http.responseStatusCode()==200)
            {
              isFirstTime=false;  
            }else {
              reset();
            } 
            http.stop();
        }else 
        {
          http.get("/gl?password=" + password);
          LOCK = (http.responseStatusCode() == 200) ?   http.responseBody() : "off";
          if(LOCK == "on")
          {
          digitalWrite(12, LOW);
          if(digitalRead(5) == LOW) 
            {
            sendRequest("/send-notification?alarm=on&password" + password);
            digitalWrite(13, LOW);
            }
          }else if(LOCK == "off") {
            digitalWrite(12, HIGH);
            digitalWrite(13,HIGH);
            digitalWrite(5, HIGH);
           }
          http.stop(); 
          sendRequest("/set?latitude="+String(gps.location.lat())+"&longtitude="+String(gps.location.lng()) + "&password=" + password);
          delay(1000);
          sendRequest("/set?ontime=" +String(millis()/1000)+ "&password=" + password);
        }
      }
    }
  

}

void sendRequest(String path) {
   http.get(path);
   http.stop();
}

void powerSIM(){
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
  }
