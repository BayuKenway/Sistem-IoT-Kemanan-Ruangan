#include <Arduino.h>
#include <MFRC522.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
//inisialisasi tag rfid
#define SS_PIN D8
#define RST_PIN D3
MFRC522 mfrc522(SS_PIN, RST_PIN);
#define relay D4
#define buzzer D2
//inisialisasi nama jaringan
const char *ssid = "aja konek";
const char *password = "12345677";
//variabel untuk send data
String getdata, Link;
String CardID = "";
//id ruangan dapat dilihat di database
String deviceID = "1"; //untuk identitas ruangan, tidak boleh ada spasi !
void inisialisasi()
{
  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(relay, HIGH);
}
void beep_success()
{
  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  delay(300);
  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  delay(300);
}
void beep_error()
{
  for (int i = 0; i < 30; i++)
  {
    digitalWrite(buzzer, HIGH);
    delay(30);
    digitalWrite(buzzer, LOW);
    delay(30);
  }
}
void check_masterkey()
{
  String masterkey = "83126203";
  String key_scanned;
  if (mfrc522.PICC_IsNewCardPresent())
  {
    mfrc522.PICC_ReadCardSerial();
    for (byte a = 0; a < mfrc522.uid.size; a++)
    {
      key_scanned += mfrc522.uid.uidByte[a];
    }
  }
  if (key_scanned == masterkey)
  {
    Serial.println("Kunci Master");
    digitalWrite(relay, LOW);
    beep_success();
    delay(5000);
    digitalWrite(relay, HIGH);
  }
}
void setup()
{
  inisialisasi();
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  WiFi.mode(WIFI_OFF); //di off kan dulu supaya mereset status
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to your WiFi router
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(ssid);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    check_masterkey();
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /*
  Catatan penting untuk pin D4, bahwasanya ketika diberi LOW justru bernilai 1 / nyala
  namun apabila diberi HIGH dia akan bernilai 0 atau false
  dia bernilai 1 saat booting jadi tiati ada bug disana
  */
}

void loop()
{
  //kalo disconnect dari jaringan
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.print("Reconnecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    // selama belum konek
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print("."); //tampilkan titik2
      check_masterkey(); //periksa apakah ada kunci master saat offline
    }
    Serial.println("");
    Serial.println("Connected"); //kalo konek
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); //IP device diberikan
  }
  check_masterkey();
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    //kalo ga ada kartu terdeteksi, maka kembalikan ke awal loop
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial())
  {
    //kalo tidak menemukan uid kartu, kembalikan ke awal
    return;
  }
  //jika menemukan uid kartu maka masukan ke variabel CardID
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    CardID += mfrc522.uid.uidByte[i];
  }
  //memanggil http
  WiFiClient client;
  HTTPClient httpclient;
  Link = "http://192.168.43.136/wpu-login/Upload/masuk?CardID=" + CardID + "&deviceID=" + deviceID;
  httpclient.begin(client, Link);
  int httpcode = httpclient.GET();
  String response = httpclient.getString();
  Serial.println(CardID);
  Serial.println(httpcode);
  Serial.println(response);
  if (response == "login")
  {
    Serial.println("Selamat Datang");
    digitalWrite(relay, LOW);
    beep_success();
    // digitalWrite(buzzer,HIGH);
    delay(5000); //buka kunci selama 5 detik
  }
  else if (response == "gagal")
  {
    beep_error();
  }
  delay(500);
  CardID = "";
  getdata = "";
  Link = "";
  httpclient.end();
  digitalWrite(buzzer, LOW);
  digitalWrite(relay, HIGH);
}
