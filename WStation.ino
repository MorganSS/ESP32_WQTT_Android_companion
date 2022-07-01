#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

int valTemp, valHumi, valPres, valAlti;
int ledPin = 27;
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

//CONNECTION
const char* ssid = "Keenetic-5031";
const char* password = "ft8PM7Nt";

//define mqtt
const char* mqttServerIP = "m9.wqtt.ru";
const int mqttPort = 12146;
const char* mqtt_user = "colledge";
const char* mqtt_password = "GiF13lyb";
WiFiClient espClient;
PubSubClient client(espClient);

//define topic
char* OneTopic = "kws/sensorKws";
char* TwoTopic = "kws/sensorKws1";
char* ThreeTopic = "kws/sensorKws2";
char* FourTopic = "kws/sensorKws3";

void reconnect(){
  // MQTT Begin
  while(!client.connected()){
    Serial.println("Connecting to MQTT Server..");
    Serial.print("IP MQTT Server : "); Serial.println(mqttServerIP);
    String clientId = "ESP32-" + WiFi.macAddress();
    bool hasConnection = client.connect(clientId.c_str(), mqtt_user, mqtt_password);
    if(hasConnection){
      Serial.println("Success connected to MQTT Broker");
    } else {
      Serial.print("Failed connected");
      Serial.println(client.state());
      digitalWrite(4,HIGH);
      delay(2000);
      Serial.println("Try to connect...");
    }
  }
  client.publish(OneTopic, "Reconnecting");
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.println("--------");
  Serial.println("Message Arrived");
  Serial.print("Topic :"); Serial.println(topic);
  Serial.print("Message : ");
  String pesan = "";
  for(int i=0; i < length; i++){
    Serial.print((char)payload[i]);
    pesan += (char)payload[i];
  }
  if(pesan == "ON" ){
    Serial.println("LED ON.. Warning");
    digitalWrite(4,HIGH);
  } else if(pesan == "OFF"){
    Serial.println("LED OFF.. Safety");
    digitalWrite(4,LOW);
  }
  Serial.println("ESP/CMD topic arrived");
  Serial.println(pesan);
  Serial.print("Pesan masuk :");
  Serial.println(pesan);
  Serial.println("--------");
}

//BME280
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;


 
void setup() {
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  pinMode(4,OUTPUT);
  
  Serial.begin(115200);
  if (!bme.begin()) {
    Serial.println("BME280 no detected");
    while (1);
  }
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServerIP, mqttPort);
  client.setCallback(callback);
  delay(20);
}

void Connection(){
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()){
    client.connect("Client");
  }
}

char dataPublish[50];
void publishMQTT(char* topics, String data){
   data.toCharArray(dataPublish, data.length() + 1);
   client.publish(topics, dataPublish);
}

int ledState = 0,ledStateN = HIGH;
unsigned long previousMillis = 0,previousMillisStream = 0;
const long interval = 1000,intervalStream = 500; 

void ledBlink(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == 0) {
      ledState = 16 ;
    } else {
      ledState = 0;
    }
    ledcWrite(ledChannel, ledState);
  }
}

void dataStream(){
  unsigned long currentMillisStream = millis();
  if (currentMillisStream - previousMillisStream >= intervalStream) {
    previousMillisStream = currentMillisStream;
    //publishMQTT(OneTopic,(String(valTemp) + "," + String(valHumi) + "," + String(valPres) + "," + String(valAlti)));
    publishMQTT(OneTopic,(String(valTemp)));
    publishMQTT(TwoTopic,(String(valHumi)));
    publishMQTT(ThreeTopic,(String(valPres)));
     publishMQTT(FourTopic,(String(valAlti)));
    Serial.println("Temp: " + String(valTemp) + " *C | Humi: " + String(valHumi) + " % | Pres: " + String(valPres) + " hPa | Alti: " + String(valAlti) + " m" );
  }
}

void loop() {
  Connection();
   
  
  valTemp = bme.readTemperature();
  valHumi = bme.readHumidity();
  valPres = bme.readPressure() / 100.0F;
  valAlti = bme.readAltitude(SEALEVELPRESSURE_HPA);

  ledBlink();
  
  dataStream();
  
  delay(50);
}
