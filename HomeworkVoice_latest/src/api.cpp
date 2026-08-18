#include "api.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <esp_heap_caps.h>
#include <mbedtls/base64.h>
#include "config.h"
HomeworkApiClient::HomeworkApiClient(const char*u):base_(u){}
ApiResult HomeworkApiClient::Post(const String&p,const String&b){ApiResult r;for(uint8_t a=1;a<=config::kHttpRetries;a++){WiFiClient c;HTTPClient h;h.setConnectTimeout(config::kHttpTimeoutMs);h.setTimeout(config::kHttpTimeoutMs);if(!h.begin(c,base_+p)){r.error="HTTP初期化失敗";continue;}h.addHeader("Content-Type","application/json; charset=utf-8");int code=h.POST(b);r.status_code=code;if(code>0){String x=h.getString();r.ok=code>=200&&code<300;JsonDocument d;if(!deserializeJson(d,x)){r.response_text=d["response_text"]|"";r.transcript=d["transcript"]|"";r.audio_url=d["audio_url"]|"";if(!r.ok)r.error=d["detail"]|x;}else r.error=x;h.end();if(r.ok||code<500)return r;}else{r.error=h.errorToString(code);h.end();}delay(300*a);}return r;}
ApiResult HomeworkApiClient::SendVoiceAudio(const uint8_t*w,size_t z,const String&s){size_t n=0;mbedtls_base64_encode(nullptr,0,&n,w,z);char*b=static_cast<char*>(heap_caps_malloc(n+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));ApiResult r;if(!b){r.error="Base64メモリ不足";return r;}if(mbedtls_base64_encode(reinterpret_cast<unsigned char*>(b),n,&n,w,z)){heap_caps_free(b);r.error="Base64失敗";return r;}b[n]=0;String body="{\"audio_base64\":\"";body+=b;body+="\",\"session_id\":\""+s+"\",\"sample_rate\":16000}";heap_caps_free(b);return Post("/voice/audio",body);}
ApiResult HomeworkApiClient::CreateTts(const String&t){JsonDocument d;d["text"]=t;String b;serializeJson(d,b);return Post("/tts",b);}
bool HomeworkApiClient::DownloadAudio(const String&u,uint8_t**d,size_t*z,String*e){WiFiClient c;HTTPClient h;h.setTimeout(config::kHttpTimeoutMs);if(!h.begin(c,base_+u)){*e="音声接続失敗";return false;}int code=h.GET(),n=h.getSize();if(code!=200||n<=0||n>2097152){*e="音声取得失敗";h.end();return false;}*d=static_cast<uint8_t*>(heap_caps_malloc(n,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));if(!*d){*e="メモリ不足";h.end();return false;}size_t got=h.getStreamPtr()->readBytes(*d,n);h.end();if(got!=size_t(n)){heap_caps_free(*d);*d=nullptr;*e="受信不足";return false;}*z=got;return true;}
