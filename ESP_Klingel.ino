#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <tr064.h>

const char WIFI_SSID[] = "Mein_WLAN";                // <-- ändern
const char WIFI_PASSWORD[] = "ganz_geheim";          // <-- ändern
const char USER[] = "Admin";                         // <-- ändern
const char PASSWORD[] = "geheim";                    // <-- ändern
const char FRITZBOX_IP[] = "192.168.178.1";          // <-- ändern
const int FRITZBOX_PORT = 49000;
const char* IOBROKER = "192.168.178.99";             // <-- ändern auf ioBroker-IP
const int IOBROKER_PORT = 8087;
String IOBROKER_DP = "/set/javascript.0.ESP_Klingel?value="; //Datenpunkt ggf. anpassen
int KLINGELDAUER = 4;                                // wieviel Sekunden sollen die Telefone klingeln

TR064 tr064_connection(FRITZBOX_PORT, FRITZBOX_IP, USER, PASSWORD);

const IPAddress STATIC_IP(192, 168, 178, 214);       // Daten der ESP-Klingel...
const IPAddress GATEWAY(192, 168, 178, 1);
const IPAddress SUBNET(255, 255, 255, 0);
const IPAddress DNS(GATEWAY);

const char DEVICE_NAME[] = "ESP-Klingel";

void setup() {
  WiFi.hostname(DEVICE_NAME);
  WiFi.config(STATIC_IP, SUBNET, GATEWAY, DNS);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED) {
   delay(50);
  }

  tr064_connection.init();

  String tr064_service = "urn:dslforum-org:service:X_VoIP:1";

  // Die Telefonnummer **9 ist der Fritzbox-Rundruf.
  String call_params[][2] = {{"NewX_AVM-DE_PhoneNumber", "**9"}};
  tr064_connection.action(tr064_service, "X_AVM-DE_DialNumber", call_params, 1);


  //ioB-Part Datenpunkt per Simple-API setzen##############
   // Use WiFiClient class to create TCP connections
      WiFiClient client;
      client.connect(IOBROKER, IOBROKER_PORT);
      if (!client.connect(IOBROKER, IOBROKER_PORT)) {
         Serial.println("connection failed");
      return;
      }
      client.print(String("GET ") + IOBROKER_DP + "true HTTP/1.1\r\n" +
               "Host: " + IOBROKER + "\r\n" +
               "Connection: close\r\n" +
               "\r\n"
              );
      client.stop();
  //#######################################################

  // Warte x Sekunden bis zum auflegen
  delay(KLINGELDAUER*1000);
  tr064_connection.action(tr064_service, "X_AVM-DE_DialHangup");

  //ioB-DP löschen
  client.connect(IOBROKER, IOBROKER_PORT);
  if (!client.connect(IOBROKER, IOBROKER_PORT)) {
     Serial.println("connection failed");
  return;
  }
  client.print(String("GET ") + IOBROKER_DP + "false HTTP/1.1\r\n" +
           "Host: " + IOBROKER + "\r\n" +
           "Connection: close\r\n" +
           "\r\n"
          );
  client.stop();

  ESP.deepSleep(0);
}

void loop() {}
