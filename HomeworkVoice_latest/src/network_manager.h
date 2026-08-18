#pragma once
#include <Arduino.h>
class NetworkManager{public:NetworkManager(const char*,const char*);void Begin();void Update();bool IsConnected()const;private:void Connect();const char*ssid_;const char*password_;uint32_t last_=0;};
