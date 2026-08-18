#pragma once
#include <Arduino.h>
struct ApiResult{bool ok=false;int status_code=0;String response_text,transcript,audio_url,error;};
class HomeworkApiClient{public:explicit HomeworkApiClient(const char*);ApiResult SendVoiceAudio(const uint8_t*,size_t,const String&);ApiResult CreateTts(const String&);bool DownloadAudio(const String&,uint8_t**,size_t*,String*);private:ApiResult Post(const String&,const String&);String base_;};
