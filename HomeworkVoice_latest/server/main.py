import base64,hashlib,subprocess,tempfile,threading
from pathlib import Path
from fastapi import FastAPI,HTTPException
from fastapi.responses import FileResponse
from pydantic import BaseModel,Field
from faster_whisper import WhisperModel
app=FastAPI(title='HomeworkVoice');lock=threading.Lock();model=None;tts_dir=Path(tempfile.gettempdir())/'homework_tts';tts_dir.mkdir(exist_ok=True)
class Audio(BaseModel):audio_base64:str=Field(min_length=60);session_id:str='default';sample_rate:int=16000
class Tts(BaseModel):text:str
@app.get('/health')
def health():return {'status':'ok'}
def make_tts(text):
 ident=hashlib.sha256(text.encode()).hexdigest()[:24];wav=tts_dir/f'{ident}.wav'
 if not wav.exists():
  aiff=tts_dir/f'{ident}.aiff';subprocess.run(['say','-v','Kyoko','-o',str(aiff),text],check=True);subprocess.run(['afconvert','-f','WAVE','-d','LEI16@16000','-c','1',str(aiff),str(wav)],check=True);aiff.unlink(missing_ok=True)
 return ident
@app.post('/tts')
def tts(p:Tts):return {'audio_url':f'/tts/{make_tts(p.text)}.wav'}
@app.get('/tts/{ident}.wav')
def wav(ident:str):
 p=tts_dir/f'{ident}.wav'
 if not p.exists():raise HTTPException(404,'not found')
 return FileResponse(p,media_type='audio/wav')
@app.post('/voice/audio')
def voice(p:Audio):
 global model
 data=base64.b64decode(p.audio_base64);f=tempfile.NamedTemporaryFile(suffix='.wav',delete=False);f.write(data);f.close()
 with lock:
  if model is None:model=WhisperModel('small',device='cpu',compute_type='int8')
  seg,_=model.transcribe(f.name,language='ja',temperature=0.0,vad_filter=True);text=''.join(s.text for s in seg).strip()
 Path(f.name).unlink(missing_ok=True)
 response=f'認識しました。{text}' if text else '音声を認識できませんでした'
 return {'intent':'diagnostic','transcript':text,'response_text':response,'session_id':p.session_id,'audio_url':f'/tts/{make_tts(response)}.wav'}
