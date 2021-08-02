/*
 * PROJETO INTEGRADOR 6 - IFSC
 *
 *   ---  SafeBox  ------
 *
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
#include <SPI.h>
#include <MFRC522.h>

// Network setings
const char* ssid = "Junges-2.0";              //wifi SSID
const char* password = "junges2402";          // wifi password
const char* mqtt_server = "161.35.1.122";     //host mqtt
int temps = 0;                                //varible for count attempts conection to wifi
int temps2 = 0;                               //varible for count attempts conection to mqtt

// Default init configs pi 2
int pino_passo = 15;        // Step pin
int pino_direcao = 2;      // Direction pin
const int pino_chave1 = 4;   // End of course close pin
const int pino_chave2 = 16;   // End of course open pin
int direcao = 1;            // Direction variable inicialize
int pos = 0;                //servo motor posicion variable inicialize

Servo myservo;  // create servo object to control a servo
WiFiClient espClient;
PubSubClient client(espClient);

// default configs lcd

#define endereco  0x27 // Endereços comuns: 0x27, 0x3F
#define colunas   16
#define linhas    2
LiquidCrystal_I2C lcd(endereco, colunas, linhas);
//end default configs lcd

// default configs keypad

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns

char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[ROWS] = {12, 17, 5, 26};
byte colPins[COLS] = {25, 33, 32};
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define Password_Length 7
char Data[Password_Length];
char Master[Password_Length] = "123456";
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;
//end default configs keypad

//default confis rfid

#define SS_PIN 14
#define RST_PIN 27
MFRC522 mfrc522(SS_PIN, RST_PIN);
//end default configs rfid

void setup_serial(){
  Serial.begin(9600);
}

void setup_wifi() {        // connecting to a WiFi network
   delay(100);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.println(password);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED and temps < 10)  // try 10 times to connect wifi, if not possivel, skip
    {
      delay(500);
      Serial.print("...");
      temps+=1;
    }
  randomSeed(micros());
  if (WiFi.status() == WL_CONNECTED){
    lcd.print("Wifi connected");
  }
  Serial.println("");
  Serial.println("WiFi status: ");
  Serial.print(WiFi.status());
  Serial.println(WiFi.localIP());
}

void setup_lcd(){
  lcd.init(); // init display comunication
  lcd.backlight(); // turn on backlight
  lcd.print("Inicializando...");
  delay(2000);
  lcd.clear();
}

void setup_rfid(){
  SPI.begin();   // start comunication SPI to the RFID
  mfrc522.PCD_Init();  //init RFID module
  mfrc522.PCD_DumpVersionToSerial(); // debug mfrc version firmware
}

void callback(char* topic, byte* payload, unsigned int length) // void to handle the mqtt data
{

  Serial.println("Command from MQTT broker is :   ");

  int location=String((char*)payload).toInt();
  Serial.println(location);

  if(location==1) {                                // open box
    client.publish("status/box","Box opening...");
    if (myservo.read() != 0){
      myservo.write(0);
      delay(1000);
    }

    direcao = 1;                                  // Define rotation direction
    digitalWrite(pino_direcao, direcao);
    while(digitalRead(pino_chave2) == HIGH){
      digitalWrite(pino_passo, 1);
      delay(1);
      digitalWrite(pino_passo, 0);
      delay(1);
    }
    client.publish("status/box","Box open");
  } //end condition 1

  else if(location==2) {                          // close box
    client.publish("status/box","Box closing...");
    if (myservo.read() != 0){
      myservo.write(0);
      delay(1000);
    }
    direcao = 0;                                  // reverse rotation direction
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
  client.publish("status/box","Box close");

  } //end of condition 2

  else if(location==3) {                        //lock motor
   myservo.write(85);

  }

  else if(location==4) {                        //unlock motor
   myservo.write(0);

  }
}//end callback

void reconnect() {     // Loop until we're reconnected to mqtt server, try 3 times

  while (!client.connected() and temps2 < 3)
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command on motor
      client.subscribe("motor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
    temps2+=1;
  }
} //end reconnect()

void clearData(){     //clear data keypad
  while(data_count !=0){
    Data[data_count--] = 0;
  }
  return;
}

void setup() {
  //define outputs pins
  pinMode(pino_passo, OUTPUT);
  pinMode(pino_direcao, OUTPUT);
  pinMode(pino_chave1, INPUT_PULLUP);
  pinMode(pino_chave2, INPUT_PULLUP);
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object

  setup_serial();
  setup_lcd();
  setup_rfid();
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if(digitalRead(pino_chave1) == HIGH) //test if servo is open on the init
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
} //end setup

void loop() {

  if (!client.connected()) { //call reconnect funcition.
  reconnect();

  }

  if (!client.connected()) {

    //Serial.print(mfrc522.PICC_IsNewCardPresent());

    if (mfrc522.PICC_IsNewCardPresent() and mfrc522.PICC_ReadCardSerial()) {   //test if there is any tag present
      Serial.print("new tag is present");
      String conteudo = "";      // inicilizate string to store tag content

      Serial.println("id da tag :");

      for (byte i = 0; i < mfrc522.uid.size; i++){       // verify bits on card memory
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); //concatenate tag infos
        conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println();
      conteudo.toUpperCase();                      // upper case conteudo

      if (conteudo.substring(1) == "54 67 ED F7"){  //test if presented tag is the correct tag
        Serial.println("Tag correta, abrindo caixa");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Tag correta");
        delay(3000);
        if(digitalRead(pino_chave1) == LOW) {                                // open box

          if (myservo.read() != 0){
            myservo.write(0);
            delay(1000);
          }

          direcao = 1;                                  // define rotation direction
          digitalWrite(pino_direcao, direcao);
          while(digitalRead(pino_chave2) == HIGH){
            digitalWrite(pino_passo, 1);
            delay(1);
            digitalWrite(pino_passo, 0);
            delay(1);
          }
        } //end condition 1

        else if(digitalRead(pino_chave1) == HIGH) {                          // close box
          if (myservo.read() != 0){
            myservo.write(0);
            delay(1000);
          }
          direcao = 0;                                  // reverse rotation direction
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

      }
      else{
        lcd.clear();
        Serial.println("Tag incorreta");
        lcd.setCursor(0,0);
        lcd.print("Tag incorreta");
        delay(3000);
      }
    }

    customKey = customKeypad.getKey(); //get the keys

    lcd.setCursor(0,0);
    lcd.print("Password or tag");

    //Serial.print(customKey);

    if (customKey){
      Data[data_count] = customKey;
      lcd.setCursor(data_count,1);
      lcd.print(Data[data_count]);
      data_count++;
    }

    if(data_count == Password_Length-1){ //verify if pass is enough length
      lcd.clear();

      if(!strcmp(Data, Master)){  //compair pass
        lcd.print("Correct");
        if(digitalRead(pino_chave1) == LOW) {                                // open box

          if (myservo.read() != 0){
            myservo.write(0);
            delay(1000);
          }

          direcao = 1;                                  // Define rotation direction
          digitalWrite(pino_direcao, direcao);
          while(digitalRead(pino_chave2) == HIGH){
            digitalWrite(pino_passo, 1);
            delay(1);
            digitalWrite(pino_passo, 0);
            delay(1);
          }
        } //end condition 1

        else if(digitalRead(pino_chave1) == HIGH) {                          // close box
          if (myservo.read() != 0){
            myservo.write(0);
            delay(1000);
          }
          direcao = 0;                                  // reverse rotation direction
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
