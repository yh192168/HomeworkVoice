#include "network_manager.h"
#include <WiFi.h>
#include "config.h"
NetworkManager::NetworkManager(const char*s,const char*p):ssid_(s),password_(p){}
void NetworkManager::Begin(){WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);WiFi.persistent(false);Connect();}
void NetworkManager::Update(){if(!IsConnected()&&millis()-last_>=config::kWifiReconnectIntervalMs)Connect();}
bool NetworkManager::IsConnected()const{return WiFi.status()==WL_CONNECTED;}
void NetworkManager::Connect(){last_=millis();WiFi.disconnect(false,false);WiFi.begin(ssid_,password_);Serial.printf("[INFO] Wi-Fi接続中: %s\n",ssid_);uint32_t s=millis();while(!IsConnected()&&millis()-s<config::kWifiConnectTimeoutMs){delay(250);Serial.print('.');}Serial.println();if(IsConnected())Serial.printf("[INFO] Wi-Fi接続完了: %s\n",WiFi.localIP().toString().c_str());else Serial.println("[WARNING] Wi-Fi接続タイムアウト");}
