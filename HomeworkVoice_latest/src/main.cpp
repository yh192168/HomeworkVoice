#include <Arduino.h>
#include <esp_heap_caps.h>
#include "api.h"
#include "audio.h"
#include "config.h"
#include "network_manager.h"
NetworkManager network(config::kWifiSsid,config::kWifiPassword);HomeworkApiClient api(config::kApiBaseUrl);VoiceS3rAudio audio;String session_id;bool previous=HIGH;
void SpeakUrl(const String&u){uint8_t*d=nullptr;size_t z=0;String e;if(api.DownloadAudio(u,&d,&z,&e)){audio.PlayWav(d,z);heap_caps_free(d);}else Serial.printf("[WARNING] %s\n",e.c_str());}
void Send(){if(!network.IsConnected())return;Serial.println("[INFO] Whisperへ送信中");ApiResult r=api.SendVoiceAudio(audio.WavData(),audio.WavSize(),session_id);if(!r.ok){Serial.printf("[ERROR] API %d: %s\n",r.status_code,r.error.c_str());return;}Serial.printf("[INFO] 文字起こし: %s\n",r.transcript.c_str());Serial.printf("[INFO] 応答: %s\n",r.response_text.c_str());if(!r.audio_url.isEmpty())SpeakUrl(r.audio_url);}
void setup(){Serial.begin(115200);delay(2000);pinMode(config::kUserButtonPin,INPUT_PULLUP);Serial.println("[INFO] HomeworkVoice VoiceS3R起動");if(!audio.Begin())Serial.println("[ERROR] ES8311音声初期化失敗");else audio.TestSpeaker();network.Begin();session_id="atom-"+String(uint32_t(ESP.getEfuseMac()),HEX);Serial.println("[INFO] 待機。正面ボタンを押して話してください");}
void loop(){network.Update();bool b=digitalRead(config::kUserButtonPin);if(previous==HIGH&&b==LOW&&!audio.IsRecording()){Serial.println("[INFO] ボタン押下");if(!audio.StartRecording())Serial.println("[ERROR] 録音開始失敗");}if(audio.IsRecording())audio.UpdateRecording();if(audio.IsRecording()&&((previous==LOW&&b==HIGH)||audio.ShouldAutoStop())){Serial.println("[INFO] 録音停止");if(audio.StopRecording())Send();else Serial.println("[WARNING] 音声が小さいか短すぎます");}previous=b;delay(5);}
