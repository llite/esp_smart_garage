/*
This program allows for wireless control of a garage door.
It uses HA(home-assistant) for the control and automation, using MQTT as a broker between home-assistant and the ESP
The 'button' press to trigger the door is handled using a script in HA, that sends an 'on' payload then 1 second later and 'off' payload
*/

//libraries//
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//circuit info//
const byte RL = 14;
const byte ML = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);

//wifi info//
const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
WiFiClient espClient;

//MQTT info//
const char* mqtt_server = "MQTT_ip";
PubSubClient client(espClient);
String garage_door;
String strTopic;
String strPayload;
String door_trig = "ha/garage/door";

//WIFI Setup///
void setup_wifi() {

  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    lcd.setCursor(14, 1);
    lcd.print("\\");
    digitalWrite(ML, HIGH);
    delay(500);
    Serial.print(".");
    lcd.print("|");
    digitalWrite(ML, LOW);
    delay(500);
    Serial.print(".");
    lcd.print("/");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);
}

//MQTT function//
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);
  if (strTopic == door_trig) 
  {
    garage_door = String((char*)payload);
    if (garage_door == "on")
    {
      Serial.println("Active");
      lcd.clear();
      lcd.print("Door Active");
      digitalWrite(RL, HIGH);
    }
    else 
    {
      Serial.println("off");
      digitalWrite(RL, LOW);
      lcd.clear();
      lcd.print("Door Inactive");
    }
  }
}

//Wifi and MQTT reconnect  function//
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection..."); 
    if (client.connect("arduinoClient")) { 
      Serial.println("connected");  
      client.subscribe("ha/#"); 
    } else {
      Serial.print("failed, state=");  
      Serial.print(client.state()); 
      Serial.println(" try again in 5 seconds");  
      delay(5000);
    }
  }
}


void setup() {
  lcd.begin();  
  pinMode(RL, OUTPUT);  
  pinMode(ML, OUTPUT);  
  Serial.begin(9600); 
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);  
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) {  
    reconnect();
  }
  if (client.connected() == true) { 
    digitalWrite(ML, HIGH);
  }
      if (client.connected()==false){  
    digitalWrite(ML,LOW);
  }
  client.loop();  
}
