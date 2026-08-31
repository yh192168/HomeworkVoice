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

M5Atomic-EchoBase（AtomS3R）ベース：スタンドアロン型宿題管理ツール開発ロードマップ

1. プロジェクト概要と目的

本プロジェクトは、M5Atomic-EchoBase（AtomS3R）を中核とし、PCや外部ローカルサーバーを一切介在させない「完全スタンドアロン型」の宿題管理システムを構築することを目的とします。

従来、音声解析やAPI連携にはPython等の外部サーバーが介在していましたが、本ロードマップではデバイスから直接Google Apps Script（GAS）およびWit.aiと連携するアーキテクチャへ刷新します。ユーザーが既存のクラウドエコシステムを活用することで、追加コストを抑えつつ、高度な自然言語処理を用いた宿題管理を「簡単に」導入・運用できることをゴールとします。

2. 全体システム構成図（アーキテクチャ）

本システムは、GASを「オーケストレーター（プロキシ）」として中央に配置し、以下のデータフローで動作します。

1. デバイス層（M5Atomic-EchoBase）: ES8311による集音。録音データをPSRAMにバッファリングし、GASのRESTfulエンドポイントへHTTPS POST（Chunked Transfer Encoding）で送信。
2. ロジック・統合層（GAS / Proxy）: デバイスからのバイナリデータを受信。GASがプロキシとなりWit.ai APIを叩き、音声解析を実行。
3. 認識・解析層（Wit.ai）: STT（音声テキスト化）およびNLP（自然言語解析）を実行。「明日」などの相対表現をISO 8601形式の絶対日付へ変換し、GASに返却。
4. データ保存・可視化層（Google サービス）:
  * Google カレンダー: 解析された日付に基づき、締め切りイベントを自動登録。
  * Google スプレッドシート: ステータス管理（未完了/完了）を含む履歴を記録。

3. Wit.aiによる自然言語解析の自動化

Wit.aiの強力なNLPエンジンを活用し、日常会話に近い表現から構造化データを抽出します。

* エンティティ定義:
  * wit/datetime: 「明日」「来週の月曜」などの曖昧な表現を自動で確定日付へ変換。
  * task_name: 宿題の具体的な内容を抽出。
* インテント設計:
  * add_homework: ユーザーが宿題を登録したいという意図を定義。
* データ変換プロセス: GAS側でWit.aiのレスポンス（JSON）をパースし、wit/datetimeから得られたタイムスタンプをGoogleカレンダーの createEvent 引数に適合する形式へ正規化します。

4. M5Stack側の実装：通信プロトコルとハードウェア制御

スタンドアロン化の肝となる、セキュアかつ効率的な通信実装の詳細です。

HTTPS通信とリダイレクト制御

HTTPClient.h を使用してGASのウェブアプリURLへ接続します。

* 重要: GASは実行時に 302 Found による一時的なURLへのリダイレクトを発生させます。実装時に必ず client.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); を有効にしてください。これを怠ると、データのPOSTに失敗します。
* SSL/TLS: 指紋認証（Fingerprint）は期限切れのリスクがあるため、Googleが使用する GlobalSign Root CA 証明書を組み込むか、開発時は client.setInsecure(); を検討してください。

オーディオ管理とPSRAMの活用

AtomS3Rの「R」モデルが持つ拡張RAM（PSRAM）を最大限に活用します。

* バッファリング: ネットワークのジッターによるパケットロスを防ぐため、録音データを一度PSRAM上にバッファ。
* 転送: Transfer-Encoding: chunked を採用し、メモリ消費を抑えつつ大容量の音声データをGASへストリーミング送信します。

ハードウェア固有設定（Source Context準拠）

* 初期化: 診断ログに PI4IOE 0x43: 未検出 と出ても、AtomS3Rでは正常な挙動です。init() の戻り値に関わらず処理を続行してください。
* アンプ制御: NS4150Bの制御（GPIO18）をLOW/HIGHで切り替え、録音・再生時のノイズ制御を行います。
* Mic RMS監視: 入力レベル（RMS値）を監視し、一定の閾値を超えたら「録音開始」、静寂が続いたら「送信開始」というステート遷移を実装します。

5. GAS（Google Apps Script）によるバックエンド構築

GASはデバイスと各APIをつなぐ「司令塔」として機能します。

* doPost(e)の実装: 受信したバイナリデータを UrlFetchApp を用いてWit.aiのSpeech APIへ転送。結果をJSONで受け取ります。
* サービス連携:
  * CalendarApp: 抽出された日付とタスク名をカレンダーに書き込み。
  * SpreadsheetApp: 管理台帳へ追記。
* レスポンス: 処理結果（成功・失敗、認識した内容）をJSON形式でデバイスへ返却し、AtomS3Rの画面表示に反映させます。

6. UI/UX：AtomS3R State Machine Design

128x128ディスプレイを用い、ユーザーに現在のシステム状態をフィードバックします。

1. IDLE: 待機状態。直近の締め切り宿題をスクロール表示。
2. LISTENING: 録音中（赤背景）。Mic RMSに連動した波形を表示。
3. PROCESSING: クラウド解析中（青背景）。「Processing...」と表示し、二重送信を防止。
4. RESULT_CONFIRM: 確認状態。GASから返ってきた「内容」と「日付」を表示。本体ボタン押下で「確定・保存」を実行するインターロック機構。

7. 開発フェーズとマイルストーン

* Phase 1: Cloud Foundation: Wit.aiのインテント学習、およびGASの doPost 基本テンプレート作成。
* Phase 2: Secure Connectivity: M5StackからGASへのHTTPS接続テスト。リダイレクト処理と証明書検証の安定化。
* Phase 3: Stream Integration: 音声バイナリの送信からWit.ai解析、カレンダー登録までの一気通貫テスト。
* Phase 4: UX Polish: PSRAMバッファリングの実装。AtomS3R画面での4状態遷移とボタン確認フローの実装。

8. 制約事項とトラブルシューティング

* GAS制限: GASの doPost には30秒の実行制限があります。音声が長すぎる場合やWit.aiの応答が遅い場合、タイムアウトが発生するため、音声は必要最小限（5〜10秒以内）に留めるロジックを推奨します。
* 秘匿情報の管理: src/config.h を作成し、STSSID, STAPSK, GAS_URL, WIT_AI_TOKEN を定義してください。このファイルは .gitignore に追加し、絶対にGitHubへ公開しないでください。
* メモリ管理: ESP32のヒープメモリ枯渇を避けるため、HTTPリクエスト後のクライアント・オブジェクトの適切な解放（client.end()）を徹底してください。
