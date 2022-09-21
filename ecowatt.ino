#include <Arduino.h>

#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace these with your own credentials.
#define SSID_ "XXXXXXXXXXXXXXXXX"
#define PASSWORD_ "XXXXXXXXXXXXX"

// Replace with "Basic [Your 64bit encoded credentials obtaineable from the RTE website]"
#define RTE_64BIT_CLIENT_CREDENTIALS "Basic XXXXXXXXXXXXXXXXXXXXXXX" 
#define RTE_OAUTH2_URL "https://digital.iservices.rte-france.com/token/oauth/"
#define RTE_ECOWATT_URL "https://digital.iservices.rte-france.com/open_api/ecowatt/v4/signals"
#define RTE_ECOWATT_URL_SANDBOX "https://digital.iservices.rte-france.com/open_api/ecowatt/v4/sandbox/signals"

// Pins for the LEDs, note that we're controlling 5 groups of 3 leds, so 15 LEDs with only 10 digital outputs.
#define POSITIVE_1 16
#define NEGATIVE_1 5
#define POSITIVE_2 0
#define NEGATIVE_2 4
#define POSITIVE_3 3
#define NEGATIVE_3 1
#define POSITIVE_4 14
#define NEGATIVE_4 12
#define POSITIVE_5 13
#define NEGATIVE_5 15

enum color {GREEN, ORANGE, RED, ERROR};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

ESP8266WiFiMulti wifiMulti;

void chgLight(enum color c, int pos_pin, int neg_pin)
{
  if (c == GREEN) {
    digitalWrite(pos_pin, LOW);
    digitalWrite(neg_pin, LOW);
  } else if (c == ORANGE) {
    digitalWrite(pos_pin, HIGH);
    digitalWrite(neg_pin, LOW);
  } else if (c == RED) {
    digitalWrite(pos_pin, HIGH);
    digitalWrite(neg_pin, HIGH);
  } else {
    digitalWrite(pos_pin, LOW);
    digitalWrite(neg_pin, HIGH);
  }
}

void chgLight_1(enum color c)
{
  chgLight(c, POSITIVE_1, NEGATIVE_1);
}

void chgLight_2(enum color c)
{
  chgLight(c, POSITIVE_2, NEGATIVE_2);
}


void chgLight_3(enum color c)
{
  //Comment this out for Serial logging.
  chgLight(c, POSITIVE_3, NEGATIVE_3);
}

void chgLight_4(enum color c)
{
  chgLight(c, POSITIVE_4, NEGATIVE_4);
}

void chgLight_5(enum color c)
{
  chgLight(c, POSITIVE_5, NEGATIVE_5);
}

String get_token(std::unique_ptr<BearSSL::WiFiClientSecure> &client)
{

  HTTPClient https;
  String token;
  Serial.print("[HTTPS] Begin authentication connexion...\n");
  if (https.begin(*client, RTE_OAUTH2_URL)) {
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    https.addHeader("Authorization", RTE_64BIT_CLIENT_CREDENTIALS);

    Serial.print("[HTTPS] POST OAUTH2 Authentication...\n");
    int httpCode = https.POST("");

    if (httpCode > 0) { // httpCode will be negative on error
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        StaticJsonDocument<200> auth_response;
        deserializeJson(auth_response, payload);
        token = (String) auth_response["access_token"];
        Serial.println("Got token: " + token);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return token;
}

bool get_ecowatt(std::unique_ptr<BearSSL::WiFiClientSecure> &client, String &token, DynamicJsonDocument &ecowatt_response)
{

  HTTPClient https;
  Serial.print("[HTTPS] Begin ecowatt request...\n");
  https.useHTTP10(true);
  
  // If you're testing changes, I recommand you switch to RTE_ECOWATT_URL_SANDBOX since
  // there is a limited ammount of requests you can make to the production endpoint in a short time.
  // They say it's only 1 request per 15 minutes period, I found that it was more than that, but it's not garanteed.
  if (https.begin(*client, RTE_ECOWATT_URL)) {
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    https.addHeader("Authorization", "Bearer " + token);

    Serial.print("[HTTPS] get ecowatt status...\n");
    int httpCode = https.GET();

    if (httpCode > 0) { // httpCode will be negative on error
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        WiFiClient *payload_stream = https.getStreamPtr();
        DeserializationError error = deserializeJson(ecowatt_response, *payload_stream);
        Serial.println(error.c_str());
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      return false;
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up...");

  pinMode(POSITIVE_1, OUTPUT);
  pinMode(NEGATIVE_1, OUTPUT);
  pinMode(POSITIVE_2, OUTPUT);
  pinMode(NEGATIVE_2, OUTPUT);

  //Comment these out for Serial logging.
  pinMode(POSITIVE_3, OUTPUT);
  pinMode(NEGATIVE_3, OUTPUT);

  pinMode(POSITIVE_4, OUTPUT);
  pinMode(NEGATIVE_4, OUTPUT);
  pinMode(POSITIVE_5, OUTPUT);
  pinMode(NEGATIVE_5, OUTPUT);

  Serial.println("Testing LEDs");
  delay(500);
  chgLight_1(GREEN);
  chgLight_2(GREEN);
  chgLight_3(GREEN);
  chgLight_4(GREEN);
  chgLight_5(GREEN);
  delay(500);
  chgLight_1(ORANGE);
  chgLight_2(ORANGE);
  chgLight_3(ORANGE);
  chgLight_4(ORANGE);
  chgLight_5(ORANGE);
  delay(500);
  chgLight_1(RED);
  chgLight_2(RED);
  chgLight_3(RED);
  chgLight_4(RED);
  chgLight_5(RED);
  delay(500);
  chgLight_1(ERROR);
  chgLight_2(ERROR);
  chgLight_3(ERROR);
  chgLight_4(ERROR);
  chgLight_5(ERROR);
  delay(500);

  Serial.println("Setting up NTP sync");
  timeClient.begin();

  // This is for GMT+1 but it is only valid from October 30th 2022, it should be changed to 7200 for GMT+2, I'm too lazy to make it automatic RN.
  // I'll keep it at GMT+1 only since this tool is only useful in winter when the chances for energy production issues is higher.
  timeClient.setTimeOffset(3600);

  Serial.println("Setting up Wifi connexion");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(SSID_, PASSWORD_);
}

void loop() {
  if (wifiMulti.run() == WL_CONNECTED) {
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();
    Serial.println(epochTime);
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // FYI, this is not a good practice, but I'm doing it anyway because it's simpler
    // and the data we're handling here is both public and insensitive, I don't care if someone does a MiTM attack on this tool.
    client->setInsecure();

    String token = get_token(client);

    // A static JSON of this size was causing a crash when allocated on the stack, which is why I'm allocating it dynamically here.
    // The allocated size is 2 times the size of the payload returned by the ecowatt api.
    // This is by design as it seems to need a lot more space than the actual text json to be able to put it entirely in the underlying structure.
    DynamicJsonDocument ecowatt_response(6000);

    if (get_ecowatt(client, token, ecowatt_response)) {
      Serial.println((String) ecowatt_response["signals"][0]["jour"]);
      int pas0 = timeClient.getHours();

      // Yes, I know, this does not look good.
      int d0 = 0;
      int pas1 = pas0 + 1;
      int d1 = 0;
      if (pas1 >= 24) {
        d1 = 1;
        pas1 = pas1 - 24;
      }
      int pas2 = pas1 + 1;
      int d2 = 0;
      if (pas2 >= 24) {
        d2 = 1;
        pas2 = pas2 - 24;
      }
      int pas3 = pas2 + 1;
      int d3 = 0;
      if (pas3 >= 24) {
        d3 = 1;
        pas3 = pas3 - 24;
      }
      int pas4 = pas3 + 1;
      int d4 = 0;
      if (pas4 >= 24) {
        d4 = 1;
        pas4 = pas4 - 24;
      }

      int color1 = (int) ecowatt_response["signals"][d0]["values"][pas0]["hvalue"] - 1;
      int color2 = (int) ecowatt_response["signals"][d1]["values"][pas1]["hvalue"] - 1;
      int color3 = (int) ecowatt_response["signals"][d2]["values"][pas2]["hvalue"] - 1;
      int color4 = (int) ecowatt_response["signals"][d3]["values"][pas3]["hvalue"] - 1;
      int color5 = (int) ecowatt_response["signals"][d4]["values"][pas4]["hvalue"] - 1;
      Serial.println(color1);
      Serial.println(color2);
      Serial.println(color3);
      Serial.println(color4);
      Serial.println(color5);

      chgLight((color) color1, POSITIVE_1, NEGATIVE_1);
      chgLight((color) color2, POSITIVE_2, NEGATIVE_2);
      chgLight((color) color3, POSITIVE_3, NEGATIVE_3);
      chgLight((color) color4, POSITIVE_4, NEGATIVE_4);
      chgLight((color) color5, POSITIVE_5, NEGATIVE_5);
      Serial.println("Wait 30 minutes before next round...");
      unsigned long delay_time = 1000L * 60 * 30;
      delay(delay_time);
    } else {
      delay(30000);
    }
  }
  delay(30000);
}
