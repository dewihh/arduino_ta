// LIBRARYS
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

// DEFINES
#define RST_PIN 5
#define SS_PIN 4
#define BlueLed 3

const int YellowLed = 2;
const int Buzzer = 16;

// VARIABLES
const char* SSID = "ZARA"; // rede wifi
const char* PASSWORD = "Harmoni75"; // senha da rede wifi

const char* BROKER_MQTT = "broker.hivemq.com"; // ip host broker
int BROKER_PORT = 1883; // port a do broker

const char* TOPIC_PING = "esp8266/postUid";

const char* TOPIC_PONG = "/empresas/douglaszuqueto/catraca/entrada/pong";

// PROTOTYPES
void initPins();
void initSerial();
void initRfid();
void initWiFi();
void initMQTT();

// OBJECTS
WiFiClient client;
PubSubClient MQTT(client); // instancia o mqtt
MFRC522 mfrc522(SS_PIN, RST_PIN); // instancia o rfid

// setup
void setup() {
  initSerial();
  initWiFi();
  initMQTT();
  initRfid();

  pinMode(BlueLed, OUTPUT);
  pinMode(YellowLed, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  digitalWrite(BlueLed, HIGH);
  digitalWrite(YellowLed, HIGH); 
  digitalWrite(Buzzer, HIGH); 
}

void loop() {
  if (!MQTT.connected()) {
  }
  recconectWiFi();
  MQTT.loop();

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(500);
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }

  rfidProcess();

}

// Implementasi Prototipe

void initSerial() {
  Serial.begin(115200);
}

void initRfid() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Dekatkan kartu Anda dengan pembaca ...");
  Serial.println();
}

void initWiFi() {
  delay(10);
  Serial.println("Menghubungkan ke: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    digitalWrite(BlueLed, LOW);
    digitalWrite(YellowLed, HIGH);
    digitalWrite(Buzzer, HIGH);
    delay(500);
    digitalWrite(BlueLed, HIGH);
    digitalWrite(YellowLed, LOW);
    digitalWrite(Buzzer, LOW);
    delay(500);
  }
  digitalWrite (BlueLed, HIGH); //--> Turn off the BlueLed when it is connected to the wifi router.
  digitalWrite(YellowLed, LOW);
  digitalWrite(Buzzer, LOW);
  Serial.println();
  Serial.print("Menghubungkan ke: " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());
}

// Berfungsi untuk menghubungkan ke MQTT Broker
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

//Fungsi yang menerima pesan yang diterbitkan
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println("Topic => " + String(topic) + " | Valor => " + String(message));

  Serial.flush();
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.println("Mencoba terhubung ke MQTT Broker: " + String(BROKER_MQTT));
    if (MQTT.connect("ESP8266-ESP12-E")) {
      Serial.println("Conectado");
      MQTT.subscribe("/empresas/douglaszuqueto/catraca/entrada/pong");

    } else {
      Serial.println("Sambungan kembali gagal");
      Serial.println("Mencoba menyambung kembali dalam 2 detik");
      delay(2000);
    }
  }
}

void recconectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
}

void rfidProcess()
{
  Serial.print("UID tag : ");
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  char UUID[9];
  conteudo.toCharArray(UUID, 9);
  Serial.println(conteudo);
  MQTT.publish(TOPIC_PING, UUID);
}
