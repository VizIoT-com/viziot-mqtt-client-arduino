#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <VizIoTMqttClient.h>

//Address of the leg with LED
#define LED_ESP 2

//ssid and access password for WI-FI connection
const char* ssid = "__________";
const char* password = "__________";

//Register to VizIoT.com and create a device
//VizIoT Device access key and password (can be found in the device settings)
String VizIoT_Device_key = "__________";
String VizIoT_Device_pass = "__________";

WiFiClient espClient;
PubSubClient clientMQTT(espClient);
VizIoTMqttClient clientVizIoT(clientMQTT);
long lastMsg = 0;
char msg[1000];
byte statusLed = 0;


/*---------- Sending data ----------------*/
Ticker sender;
bool isSendDataToServer;
void SendDataToServer() {isSendDataToServer = true;} 
#define INTERVAL_SEND_DATA 300 //Sending data every 5 minutes (5*60=300)
/*---------- Sending data ----------------*/



void setup()
{ 
  //enable LED control
  pinMode(LED_ESP, OUTPUT);
  digitalWrite(LED_ESP, HIGH);
  
  //Enable information output in Serial Monitor
  Serial.begin(9600);
  
  //Connecting to WI-FI
  setup_wifi();
  
  clientVizIoT.config(VizIoT_Device_key, VizIoT_Device_pass);
  clientVizIoT.listenCommand(callback);
  sender.attach(INTERVAL_SEND_DATA, SendDataToServer); // Create event of sending data every INTERVAL_SEND_DATA sec
}

//Processing of a data acquisition event
void callback(String parameter, byte value) {
   Serial.print("Publication of a message: parameter");
   Serial.print(parameter);
   Serial.print("value ");
    Serial.println(value);
  if (parameter.compareTo("led") == 0) {
    if (value == 1) {
      statusLed = 1;
      digitalWrite(LED_ESP, LOW);
    } else {
      statusLed = 0;
      digitalWrite(LED_ESP, HIGH);
    }

    snprintf(msg, sizeof(msg), "{\"led\":\"%c\"}", (statusLed) ? '1' : '0');
    Serial.print("Publishing a message: ");
    Serial.println(msg);

    clientVizIoT.sendJsonString(String(msg));
  }
}

//Функция подключения к WI-FI
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //Waiting for WI-FI connection
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi подключен");
}



void loop()
{
  //is required to process incoming messages and maintain connection to the Broker
  clientVizIoT.loop();

  if (isSendDataToServer) {
    isSendDataToServer = false; 
    

    snprintf (msg, sizeof(msg), "{\"rssi\":\"%i\",\"led\":\"%c\"}", WiFi.RSSI(), (statusLed) ? '1' : '0');
    Serial.print("Publishing a message: ");
    Serial.println(msg);
    clientVizIoT.sendJsonString(String(msg));
  }
}