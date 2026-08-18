#pragma once
#include <Arduino.h>
namespace config {
inline constexpr char kWifiSsid[]="52693532-2.4G";
inline constexpr char kWifiPassword[]="43954892";
inline constexpr char kApiBaseUrl[]="http://10.32.1.60:8001";
inline constexpr uint32_t kHttpTimeoutMs=30000;
inline constexpr uint8_t kHttpRetries=3;
inline constexpr uint32_t kWifiConnectTimeoutMs=15000;
inline constexpr uint32_t kWifiReconnectIntervalMs=5000;
inline constexpr int kUserButtonPin=41;
inline constexpr uint32_t kMaxRecordingMs=8000;
inline constexpr uint32_t kSilenceStopMs=1000;
inline constexpr int32_t kSilenceRms=450;
}
