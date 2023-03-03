//===SAWAH KITA===//
#include <WiFiEsp.h>
#include <ThingSpeak.h>

String ID_ALAT = "422641";
char ssid[] = "RedmiZaky";
char pass[] = "Password";

int status = WL_IDLE_STATUS;
const char* host = "api.thingspeak.com";
String request_string;

float pompa;

//===DelayUploadFor30Sec===//
unsigned long previousMillis = 0;
const long interval = 30000;

//===Database===//
WiFiEspClient client;
unsigned long myChannelNumber = 1895967;
String myWriteAPIKey = "wRiteApI";
String myReadAPIKey = "rEadApI";
unsigned int motorFieldNumber = 4;

#define AirPin A0
#define PHPin A1
unsigned long int avgValue;
int buf[10], temp;

void send_data(float airValue, float phValue, float pompa) {
  if (client.connect(host, 80)) {
    request_string = "/update?key=" + myWriteAPIKey
                     + "&field1=" + airValue
                     + "&field2=" + phValue
                     + "&field3=" + pompa;
    Serial.println(String("GET ") + request_string + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
    client.print(String("GET ") + request_string + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();
    Serial.println("Closing connection");

    client.stop();
  }
}

void setup() {
  pinMode(8, OUTPUT);
  Serial.begin(9600);
  Serial2.begin(9600);

  Serial1.begin(115200);
  WiFi.init(&Serial1);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.print("WiFi Module Tidak Ada");
    Serial2.print("wifi.txt=\"");
    Serial2.println("no wifi module");
    Serial2.print("\"");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    while (true);
  }
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial2.print("wifi.txt=\"");
    Serial2.println("Menghubungkan: ");
    Serial2.print(ssid);
    Serial2.print("\"");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1000);
  }
  Serial2.print("wifi.txt=\"");
  Serial2.println("terhubung: ");
  Serial2.print(ssid);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  delay(1000);
  printWiFiIP();
  printWiFiStatus();
  ThingSpeak.begin(client);
}

void printWiFiIP() {
  IPAddress ip = WiFi.localIP();
  Serial.println("IP kamu: ");
  Serial.print(ip);
}
void printWiFiStatus() {
  long rssi = WiFi.RSSI();
  Serial2.print("speed.txt=\"");
  Serial2.print(rssi);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

void loop() {
  //===PH===//
  for (int i = 0; i < 10; i++) //Get 10 sample value from the sensor for smooth the value
  {
    buf[i] = analogRead(PHPin);
    delay(1);
  }
  for (int i = 0; i < 9; i++) //sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for(int i=2;i<8;i++) 
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  phValue = 3.5 * phValue;
  Serial2.print("phValue.txt=\"");
  Serial2.print(phValue);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  //===PH===//

  unsigned int TinggiValue = analogRead(AirPin);
  float airValue = (TinggiValue - 800)/45.0; //map 500, 670, 0, 4
  if (TinggiValue < 800){airValue = 0;}
  float grap1 = airValue * 25;
  int grap = grap1;
  Serial2.print("airValue.val=");
  Serial2.print(grap);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  
  Serial2.print("air.txt=\"");
  Serial2.print(airValue);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  //Pompa//
  //read in field 4
  bool Pompa = ThingSpeak.readFloatField(myChannelNumber, motorFieldNumber);
  //Check the status of read operation
  int statusCode = ThingSpeak.getLastReadStatus();
  if (statusCode == 200) {
    Serial.println("Status Motor: " + String(Pompa));
  }
  else {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
  }
  
  if (Pompa == 1) {
    digitalWrite(8, HIGH);
    Serial2.print("onoff.txt=\"");
    Serial2.print("Nyala");
    Serial2.print("\"");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    pompa = 1;
  }
  else{
    if(airValue > 2){
      if(airValue >= 2.2){
        digitalWrite(8, HIGH);
        Serial2.print("onoff.txt=\"");
        Serial2.print("Nyala");
        Serial2.print("\"");
        Serial2.write(0xff);
        Serial2.write(0xff);
        Serial2.write(0xff);
        pompa = 1;
      }
    }
    else{
      digitalWrite(8, LOW);
      Serial2.print("onoff.txt=\"");
      Serial2.print("Mati");
      Serial2.print("\"");
      Serial2.write(0xff);
      Serial2.write(0xff);
      Serial2.write(0xff);
      pompa = 0;
    }
  }
  //Pompa//

  //Send Data//
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save current millis
    previousMillis = currentMillis;
    //send da data
    send_data(airValue, phValue, pompa);
  }
    
  printWiFiStatus();
}
