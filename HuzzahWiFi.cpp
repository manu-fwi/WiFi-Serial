#include <ESP8266WiFi.h>
#include "HuzzahWiFi.h"

HuzzahWiFi Huzzah;

HuzzahWiFi::HuzzahWiFi(): port(-1),server_add_type(SERVER_ADD_NONE),nb_networks(0),server(NULL)
{
  // Init all string to "" and put null terminator at the end
  WiFi_SSID[0]='\0';
  WiFi_SSID[WiFi_SSID_L]='\0';
  WiFi_PASSWD[0]='\0';
  WiFi_PASSWD[WiFi_PASSWD_L]='\0';
  remote_name[0]='\0';
  remote_name[WiFi_SERVER_NAME_L]='\0';
}

bool HuzzahWiFi::ap_connect()
{
  if (WiFi.status()==WL_CONNECTED) {
    WiFi.disconnect();
  }
  Serial.println(WiFi_SSID);
  Serial.println(WiFi_PASSWD);
  WiFi.begin(WiFi_SSID,WiFi_PASSWD);
  byte i=0;
  while ((i<=20) && (WiFi.status()!=WL_CONNECTED)) {
    i++;
    delay(500);
  }
  return WiFi.status()==WL_CONNECTED;
}


