/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * 
 * Tutorial URL 
 * CopyRight John Yu
 */
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
//#include <Servo.h>


// Update these with values suitable for your network.
const char* ssid = "Suzana";
const char* password = "20461203";
const char* mqtt_server = "161.35.227.231";


// Pinos conectados ao Step e Dir do Easydriver
int pino_passo = 17;
int pino_direcao = 18;
int direcao = 1;


//Servo myservo;  // create servo object to control a servo
WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.println("Command from MQTT broker is :   ");
  //Serial.println((char*)payload);
  
  int location=String((char*)payload).toInt();
  Serial.println(location);
  //if((location>194)||(location<0))
    //return;
    //if((int)payload[i]>194||(int)payload[i]<0)

  if(location>0)
    {
      // Define a direcao de rotacao
      direcao = 1;
      digitalWrite(pino_direcao, direcao);
      for (int p=0 ; p < location; p++)
      {
        Serial.println(p);
        digitalWrite(pino_passo, 1);
        delay(1);
        digitalWrite(pino_passo, 0);
        delay(1);
      }
    
    }
  if(location<0)
  {
  // Inverte a direcao de rotacao
  direcao = 0;
  location = -location;
  Serial.println(location);
  digitalWrite(pino_direcao, direcao);
    for (int p=0 ; p < location; p++)
    {
      Serial.println(p);
      digitalWrite(pino_passo, 1);
      delay(1);
      digitalWrite(pino_passo, 0);
      delay(1);
     }
  }

  location = 0;  
    //break;
    //myservo.write((int)payload[i]);              // tell servo to go to position in variable '(int)payload[i]'
  
}//end callback



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe("motor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //end reconnect()

void setup() {
  
  // Define os pinos como saida
  pinMode(pino_passo, OUTPUT);
  pinMode(pino_direcao, OUTPUT);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //myservo.attach(D1);  // attaches the servo on pin D1 to the servo object
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
