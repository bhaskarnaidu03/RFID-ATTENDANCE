#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
//-----------------------------------------------------------
const String web_app_url = "https://script.google.com/macros/s/AKfycbwaE0MRxuBsssPByW0JA7pdmo_ISAqB_yfB3NH-bJ8VDYuHbdunDFnnLdsJkyS8y8XTLw/exec";
//-----------------------------------------------------------
#define WIFI_SSID1 "ASUS_X00T"
#define WIFI_PASSWORD1 "987654321"
#define WIFI_SSID2 "ASUS_X00TD"
#define WIFI_PASSWORD2 "98765432"
//-----------------------------------------------------------
int blocks[] = {4,5,6,8,9,10};
#define blocks_len  (sizeof(blocks) / sizeof(blocks[0]))
//-----------------------------------------------------------
//GPIO 0 --> D3
//GPIO 2 --> D4
//GPIO 4 --> D2
#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D2
//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;      
//-----------------------------------------
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  
/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];
//-----------------------------------------
// Fingerprint for demo URL, expires on ‎Monday, ‎May ‎2, ‎2022 7:20:58 AM, needs to be updated well before this date
//const uint8_t fingerprint[20] = {0x9a, 0x87, 0x9b, 0x82, 0xe9, 0x19, 0x7e, 0x63, 0x8a, 0xdb, 0x67, 0xed, 0xa7, 0x09, 0xd9, 0x2f, 0x30, 0xde, 0xe7, 0x3c};
//9a 71 de e7 1a b2 25 ca b4 f2 36 49 ab ce f6 25 62 04 e4 3c
//-----------------------------------------





/****************************************************************************************************
 * setup() function
 ****************************************************************************************************/
void setup()
{
  //--------------------------------------------------
  /* Set BUZZER as OUTPUT */
  pinMode(BUZZER, OUTPUT);
  //--------------------------------------------------
  /* Initialize serial communications with the PC */
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  //--------------------------------------------------
  //WiFi Connectivity
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID1, WIFI_PASSWORD1);
  if (WiFi.status() != WL_CONNECTED){
    WiFi.begin(WIFI_SSID2, WIFI_PASSWORD2);
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(200);
    }
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  //--------------------------------------------------
  /* Initialize SPI bus */
  SPI.begin();
  //--------------------------------------------------
}




/****************************************************************************************************
 * loop() function
 ****************************************************************************************************/
 void loop()
{
  //------------------------------------------------------------------------
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));  
  //------------------------------------------------------------------------
  String fullURL = "", temp;
  for (byte i = 0; i < blocks_len; i++) {
    ReadDataFromBlock(blocks[i], readBlockData);
    if(i == 0){
      temp = String((char*)readBlockData);
      temp.trim();
      fullURL = "data" + String(i+1) + "=" + temp;
    }
    else{
      temp = String((char*)readBlockData);
      temp.trim();
      fullURL += "&data" + String(i+1) + "=" + temp;
    }
  }

  //Serial.println(fullURL);
  fullURL.trim();
  fullURL = web_app_url + "?" + fullURL;
  fullURL.trim();
  Serial.println(fullURL);
  //------------------------------------------------------------------------
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(3000);
  //------------------------------------------------------------------------

  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if (WiFi.status() == WL_CONNECTED) {
    //-------------------------------------------------------------------------------
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //-------------------------------------------------------------------------------
    //uncomment following line, if you want to use the SSL certificate
    //client->setFingerprint(fingerprint);
    //or uncomment following line, if you want to ignore the SSL certificate
    client->setInsecure();
    //-------------------------------------------------------------------------------
    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
    //-------------------------------------------------------------------------------

    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    if (https.begin(*client, (String)fullURL)){
      //-----------------------------------------------------------------
      // HTTP
      Serial.print(F("[HTTPS] GET...\n"));
      // start connection and send HTTP header
      int httpCode = https.GET();
      //-----------------------------------------------------------------
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
      }
      //-----------------------------------------------------------------
      else 
      {Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());}
      //-----------------------------------------------------------------
      https.end();
      delay(1000);
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    else {
      Serial.printf("[HTTPS} Unable to connect\n");
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}




/****************************************************************************************************
 * ReadDataFromBlock() function
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  //----------------------------------------------------------------------------
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //----------------------------------------------------------------------------
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  //----------------------------------------------------------------------------s
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  //----------------------------------------------------------------------------
  else {
    Serial.println("Authentication success");
  }
  //----------------------------------------------------------------------------
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //----------------------------------------------------------------------------
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");  
  }
  //----------------------------------------------------------------------------
}
