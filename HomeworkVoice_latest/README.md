# HomeworkVoice VoiceS3R 最新診断統合版

VoiceS3R内蔵ES8311をM5Atomic-EchoBaseのI2S/codec初期化で利用し、存在しないPI4IOE 0x43による`init=false`を診断表示しつつ、NS4150BをGPIO18で直接制御します。

## 設定

`src/config.h`のSSID、パスワード、Mac IPを設定します。秘密情報をGitHubへ公開しないでください。

## ビルド

pio run --target clean
pio run
pio run --target upload
pio device monitor

## サーバー

python3.11 -m venv .venv
source .venv/bin/activate
python -m pip install -r server/requirements.txt
python -m uvicorn server.main:app --host 0.0.0.0 --port 8001

## 期待ログ

ES8311 0x18: 検出
PI4IOE 0x43: 未検出（正常）
EchoBase init: 0（PI4IOEなしなら0で継続）
AMP GPIO18: LOW/HIGH
Mic RMS: 数字

## 注意

この版は公式M5Atomic-EchoBase 1.0.2の公開APIを使います。ライブラリの`init()`はPI4IOEがないためfalseになり得ますが、その前にI2SとES8311初期化が完了する公開実装を前提に継続します。まずループバック相当のビープ、RMS、Whisper送信を実機で確認してください。
