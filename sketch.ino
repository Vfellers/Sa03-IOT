#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include "DHTesp.h"

#define sensorDHT 15
DHTesp sensor;

String apiKey = "V8NIMYA3OUEQWNXC";  
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* urlApiThingSpeak = "api.thingspeak.com";
WiFiClient client;

//Variaveis para o broker mqtt:
const char* urlApiBroker = "broker.emqx.io";
int port = 1883;                              
const char* topic = "senai-vfellers";          

WiFiClient WiFiClient;

// instanciar objeto mqtt:
PubSubClient mqttClient(WiFiClient);

void conectarWifi() {
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi");
}

void conectarMqtt() {
  mqttClient.setServer(urlApiBroker, port);        //domain e server

  while(!mqttClient.connected()){
    String clientId = "aleatorio130"; 
    clientId += String(random(0xffff), HEX);     //cria aleatorio pra nao repetir id
    
    if(mqttClient.connect(clientId.c_str())){
      Serial.println("Conectado ao broker");
      mqttClient.subscribe(topic);
    }else{
      delay(500);
      Serial.println("Conectando ao Broker");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Olá");
  conectarWifi();
  sensor.setup(sensorDHT, DHTesp::DHT22);
  conectarMqtt();
}

void enviarParaThingSpeak() {
  float temperatura = sensor.getTemperature();
  float umidade = sensor.getHumidity();

  if (client.connect(urlApiThingSpeak,80))                                 //api.thingspeak.com
  {                    
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(temperatura);
    postStr += "&field2=";
    postStr += String(umidade);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  
    Serial.println("\n\nTemperatura: " + String(temperatura) + "C°");
    Serial.println("Umidade: " + String(umidade) + "%");
    Serial.println("Enviado ao Thingspeak...");
  }
}

void enviarParaMqtt() {
  float temperatura2 = sensor.getTemperature();
  float umidade2 = sensor.getHumidity();
  mqttClient.publish(topic, ("{\"temperatura\": "+ String(temperatura2) +", \"umidade\": "+ String(umidade2) +"}").c_str());
}

void loop() {
  delay(500);
  enviarParaThingSpeak();
  enviarParaMqtt();
}

