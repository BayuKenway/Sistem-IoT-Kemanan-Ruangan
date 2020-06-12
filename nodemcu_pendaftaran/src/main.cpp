#include <ESP8266WiFi.h>     //Include Esp library
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
// #include <MFRC522/src/MFRC522.h>        //include RFID library
#include <MFRC522.h>
#define SS_PIN D8 //RX slave select
#define RST_PIN D3
#define led D2

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

/* Set these to your desired credentials. */
const char *ssid = "aja konek";  //ENTER YOUR WIFI SETTINGS
const char *password = "12345677";

//Web/Server address to read/write from 
const char *host = "192.168.43.136";   //IP address of server

String getData ,Link;
String CardID="";

void setup() {
  delay(1000);
  Serial.begin(9600);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting to ");
  Serial.print(ssid);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
//
//  pinMode(RedLed,OUTPUT);
 pinMode(led,OUTPUT);
}

void loop() {
  digitalWrite(led,LOW);
  //kalo terputus
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.print("Reconnecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    // selama belum konek
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("."); //tampilkan titik2
        digitalWrite(led,HIGH);
      }
    Serial.println("");
    Serial.println("Connected"); //kalo konek
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  }
  
  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
    //got to start of loop if there is no card present
 }
 // Select one of the cards
 if ( ! mfrc522.PICC_ReadCardSerial()) {
  return;
  //if read card serial(0) returns 1, the uid struct contians the ID of the read card.
 }
//membaca id kartu
 for (byte i = 0; i < mfrc522.uid.size; i++) {
     CardID += mfrc522.uid.uidByte[i];
}
  WiFiClient client;
  HTTPClient http;    //Declare object of class HTTPClient
  String deviceID="Lab-IPA"; //ubah sesuai nama ruangan
  //GET Data
  getData = "?CardID=" + CardID;
  Link="http://192.168.43.136/wpu-login/Upload/tambahKartuBaru"+getData;
  
  http.begin(client,Link);
  
  int httpCode = http.GET();            //Send the request
  delay(10);
  String payload = http.getString();    //Get the response payload
  
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(getData);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
  Serial.println(CardID);     //Print Card ID
  
  if(payload == "Kartu Terdaftar"){
    Serial.println("Kartu Sudah Terdaftar, Ganti Kartu");
    delay(2000);  //Post Data at every 2 seconds
  }
  else if(payload == "Kartu Tersedia"){
    Serial.println("Kartu Tersedia, Dapat Digunakan");
    delay(2000);  //Post Data at every 2 seconds
  }
  else{
    Serial.println("Terjadi Kesalahan");
    delay(500);  
  }
  delay(500);
  
  CardID = "";
  getData = "";
  Link = "";
  http.end();  //Close connection 
}