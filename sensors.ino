/*
  This program allows for the wireless status monitoring of a garage
  It monitors the if the door is open, closed, and if there is a car in a spot
  It sends this info via MQTT to home-assistant
*/

//libraries//
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <HCSR04.h>

//circuit info//
LiquidCrystal_I2C lcd(0x27, 16, 2);
HCSR04 hc(12, 13);
const byte WL = 0;
const byte ML = 2;
const byte DO = 40;
const byte FD = 210;
int DI = 12;
int msg = 0;

//wifi info//
const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
const char* mqtt_server = "MQTT_ip";
WiFiClient espClient;

//MQTT info//
PubSubClient client(espClient);
const char* door_state = "ha/garage/state";
const char* door_state_bi = "ha/garage/state_bi";
const char* n_car_state = "ha/garage/n_car";
const char* n_car_state_bi = "ha/garage/n_car_bi";

//WIFI Setup//
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
    digitalWrite(WL, HIGH);
    delay(500);
    Serial.print(".");
    lcd.print("|");
    digitalWrite(WL, LOW);
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

//MQTT function//
void mqttsensor() {
  if (DI < DO ) {
    dooropen();
    maybecar();
  }
  if (DI > DO && DI < FD ) {
    car();
    doorclosed();
  }
  if (DI >= FD) {
    nocar();
    doorclosed();
  }
}

//Wifi reconnect and MQTT connection function//
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP_garage_sensor")) {
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

void dooropen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The door is open");
  lcd.setCursor(0, 1);
  lcd.print(DI);
  client.publish(door_state, "The garage door is open", true);
  client.publish(door_state_bi, "on", true);
}

void car() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("There is a car");
  lcd.setCursor(0, 1);
  lcd.print(DI);
  client.publish(n_car_state, "The northern car is in the garage", true);
  client.publish(n_car_state_bi, "on", true);
}
void nocar() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("There is no car");
  lcd.setCursor(0, 1);
  lcd.print(DI);
  client.publish(n_car_state, "There is no car in the northern spot", true);
  client.publish(n_car_state_bi, "off", true);
}
void maybecar() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The door is blocking the view");
  lcd.setCursor(0, 1);
  lcd.print(DI);
  client.publish(n_car_state, "The door is blocking the view", true);
}
void doorclosed() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The door is closed");
  lcd.setCursor(0, 1);
  lcd.print(DI);
  client.publish(door_state, "The garage door is closed", true);
  client.publish(door_state_bi, "off", true);
}

void setup() {
  lcd.begin();
  Serial.begin(9600);
  pinMode(WL, OUTPUT);
  pinMode(ML, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (client.connected() == true) {
    digitalWrite(ML, HIGH);
  }
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WL, HIGH);
  }
  if (client.connected() == false) {
    digitalWrite(ML, LOW);
  }
  if (WiFi.status() == WL_DISCONNECTED) {
    digitalWrite(WL, LOW);
  }
  client.loop();
  DI = hc.dist();
  mqttsensor();
  delay(500);
  }
