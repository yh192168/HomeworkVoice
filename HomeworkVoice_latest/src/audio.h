#pragma once
#include <Arduino.h>
#include <M5EchoBase.h>
class VoiceS3rAudio{public:VoiceS3rAudio();bool Begin();bool TestSpeaker();bool StartRecording();void UpdateRecording();bool StopRecording();bool PlayWav(const uint8_t*,size_t);bool IsRecording()const;bool ShouldAutoStop()const;const uint8_t*WavData()const;size_t WavSize()const;private:void Amp(bool);bool Probe(uint8_t);void Header();static constexpr uint32_t kRate=16000;static constexpr size_t kHead=44,kMax=kRate*2*8,kN=320;M5EchoBase echo_;uint8_t*wav_=nullptr;int32_t*io_=nullptr;size_t bytes_=0;bool ready_=false,recording_=false,voice_=false,stop_=false;uint32_t started_=0,last_voice_=0,last_log_=0;};
