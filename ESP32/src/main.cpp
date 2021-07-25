/* 
 * PROJETO INTEGRADOR 6 - IFSC
 *
 * 
 *   Author: Marcelo Junges
 *
 *  
 */


#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

// Network setings
const char* ssid = "JUNGES";
const char* password = "senha";
const char* mqtt_server = "161.35.1.122";
int temps = 0;

// Default init configs pi 1
int pino_passo = 17;        // Step pin
int pino_direcao = 18;      // Direction pin
const int pino_chave1 = 23;   // End of course close pin 
const int pino_chave2 = 22;   // End of course opem pin
int direcao = 1;            // Direction variable inicialize 
int pos = 0;                //servo motor posicion variable inicialize

Servo myservo;  // create servo object to control a servo
WiFiClient espClient;
PubSubClient client(espClient);

// default init configs lcd

#define endereco  0x27 // Endereços comuns: 0x27, 0x3F
#define colunas   16
#define linhas    2
LiquidCrystal_I2C lcd(endereco, colunas, linhas);


// default init configs keypad

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns

char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33};
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define Password_Length 8
char Data[Password_Length]; 
char Master[Password_Length] = "123456"; 
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;






void setup_serial(){
  Serial.begin(9600);
}


void setup_wifi() {
   delay(100);
  // connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED and temps < 3) 
    {
      delay(500);
      Serial.print("...");
      temps+=1;
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi status: ");
  Serial.print(WiFi.status());
  //Serial.println(WiFi.localIP());
}


void setup_lcd(){
  lcd.init(); // INICIA A COMUNICAÇÃO COM O DISPLAY
  lcd.backlight(); // LIGA A ILUMINAÇÃO DO DISPLAY
  lcd.print("Inicializando..."); 
  delay(2000); 
  lcd.clear(); // LIMPA O DISPLAY
}


void callback(char* topic, byte* payload, unsigned int length) 
{

  Serial.println("Command from MQTT broker is :   ");
  
  int location=String((char*)payload).toInt();
  Serial.println(location);

  if(location==1) {                                // Abre gaveta
    
    if (myservo.read() != 0){
      myservo.write(0);
      delay(1000);
    }

    direcao = 1;                                  // Define a direcao de rotacao
    digitalWrite(pino_direcao, direcao);
    while(digitalRead(pino_chave2) == HIGH){
      digitalWrite(pino_passo, 1);
      delay(1);
      digitalWrite(pino_passo, 0);
      delay(1);
    }
  } //end condition 1

  else if(location==2) {                          // Fecha gaveta
     if (myservo.read() != 0){
      myservo.write(0);
      delay(1000);
    }
    direcao = 0;                                  // Inverte a direcao de rotacao
    digitalWrite(pino_direcao, direcao);
    
    while (digitalRead(pino_chave1) == HIGH){
      digitalWrite(pino_passo, 1);
      delay(1);
      digitalWrite(pino_passo, 0);
      delay(1);
    }
  delay(1000);
  if (myservo.read() == 0){
      myservo.write(85);
      delay(500);
    }
 
  } //end of condition 2

  else if(location==3) {                        //Trava motor
   myservo.write(85);

  }
  
  else if(location==4) {                        //Destrava motor 
   myservo.write(0);

  }  
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

void clearData(){
  while(data_count !=0){
    Data[data_count--] = 0; 
  }
  return;
}

void setup() {
  
  // Define os pinos como saida
  pinMode(pino_passo, OUTPUT);
  pinMode(pino_direcao, OUTPUT);
  pinMode(pino_chave1, INPUT_PULLUP);
  pinMode(pino_chave2, INPUT_PULLUP);
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object

  setup_serial();
  setup_wifi();
  setup_lcd();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  Serial.print(digitalRead(pino_chave1));
  Serial.print(digitalRead(pino_chave2));



  if(digitalRead(pino_chave1) == HIGH)
  { 
    if (myservo.read() != 0){
      myservo.write(0);
      delay(1000);
    }
    while(digitalRead(pino_chave1) == HIGH)
      {
      direcao = 0;
      digitalWrite(pino_direcao, direcao);
      digitalWrite(pino_passo, 1);
      delay(1);
      digitalWrite(pino_passo, 0);
      delay(1);
      }
    
    if (myservo.read() == 0){
    delay(1000);
    myservo.write(85);    
    }
  }
  


}

void loop() {

  if (!client.connected() and temps < 3) {          
    reconnect();
    temps+=1;
  }

  if (!client.connected()) {
    customKey = customKeypad.getKey();

    lcd.setCursor(0,0);
    lcd.print("Enter Password:");
    
    if (customKey){
      Data[data_count] = customKey; 
      lcd.setCursor(data_count,1); 
      lcd.print(Data[data_count]); 
      data_count++; 
        }
    
    if(data_count == Password_Length-1){
      lcd.clear();

      if(!strcmp(Data, Master)){
        lcd.print("Correct");           //se a senha for correta abre a gaveta
        //---------------------------------------
        if(digitalRead(pino_chave1) == LOW) {                                // Abre gaveta
    
          if (myservo.read() != 0){
            myservo.write(0);
            delay(1000);
          }

          direcao = 1;                                  // Define a direcao de rotacao
          digitalWrite(pino_direcao, direcao);
          while(digitalRead(pino_chave2) == HIGH){
            digitalWrite(pino_passo, 1);
            delay(1);
            digitalWrite(pino_passo, 0);
            delay(1);
          }
        } //end condition 1

        else if(digitalRead(pino_chave1) == HIGH) {                          // Fecha gaveta
          if (myservo.read() != 0){
            myservo.write(0);
            delay(1000);
          }
          direcao = 0;                                  // Inverte a direcao de rotacao
          digitalWrite(pino_direcao, direcao);
          
          while (digitalRead(pino_chave1) == HIGH){
            digitalWrite(pino_passo, 1);
            delay(1);
            digitalWrite(pino_passo, 0);
            delay(1);
          }
        delay(1000);
        if (myservo.read() == 0){
            myservo.write(85);
            delay(500);
          }
      
        } //end of condition 2

        //----------------------------------------
        }
      else{
        lcd.print("Incorrect");
        delay(1000);
        }
      
      lcd.clear();
      clearData();  
    }
   

  }
  
  client.loop();

}


