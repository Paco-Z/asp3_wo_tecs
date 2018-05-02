                        README
           Xilnx SDK用プロジェクト（ArduZYNQ)

             2017 (C) Shinya Honda honda@ertl.jp

----------------
About
----------------
本フォルダはXilinx SDKによりZYBO向けにFMPをビルドして実行するための手
順をまとめたものである．

画面ダンプ付きの同様の手順が以下にあるため参考にすること．


----------------
Host environment
----------------
動作確認したホストOSのバージョンは次の通りである．

  - Windows 10 64bit
  
  
動作確認したXilinx SDKのバージョンは次の通りである．  

  - Xilinx SDK (>2016.3)

----------------
Board Settig
----------------
Xilinx SDK で動作させるため，ボードは次の設定とする．

  - JP5 : JTAG

----------------
Build instructions
----------------
FMPカーネルのダウンロードと展開．

ワークスペースの作成
 - Xilinx SDKを起動してワークスペースを新規に作成する．
 - File -> Import... を選択
 - General -> Existing Projects int Workspace を選択
 - Select root directoryに本フォルダを指定．
 - fmp_sample1/zybo_hwをインポート

ビルド
 - Project Exploer から fmp_sample1 を選択
 - Build Projectを選択
   - ビルドが開始される

デバッグ
 - ボードのPROG UARTとPCを接続
 - "ボードのリセットボタンを押す"
 - sample1のツリーを展開
 - sample1のツリーにある fmp_sample1.launchを右クリックして，
   Debug as -> fmp_sample1 を選択
   - 2回目以降はデバッグアイコンからから可能
 - ロード終了後 APU を選択して，Resumeボタンにより実行

------------------
Change Log
------------------
- 2017/02/18
 - 新規作成
