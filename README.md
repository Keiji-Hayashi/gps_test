# GPS test

## 実行ファイル
- **gps_test_org**<br>NMEAをそのまま表示します。
- **gps_test**<br>NMEAを解析してエラー異常が無いかチェックします。
  

## ビルド方法

```bash
mkdir build
cd build
cmake ..
make
```
  

## gps_testセットアップ手順(CE試験向け自動起動設定)
1. SDカードにSSH許可設定ファイル(devmode)をコピーしてカメラに挿入
1. SSIDを指定してWi-Fi接続
1. Windowsのコマンドプロンプトからscpコマンドででgps_tast.tar.gzをカメラの~/にコピー
    ```cmd
    > scp -i .ssh\ADVF_DEV.KEY .\gps_tast.tar.gz gasuser@192.168.11.1:~/
    ```

1. TeraTarmを使いUART経由でカメラのコンソールに接続してログイン  
    以降はカメラのコンソールで操作
1. ホワイトリスト無効化
    ```
    $ ./WhiteListSwitching.sh -d
    ```
1. gps_tast.tar.gzを解凍
    ```bash
    $ tar -zxvf gps_tast.tar.gz
    ```
1. 起動スクリプトを退避
    ```bash
    $ cp gas_all_start.sh gas_all_start.sh.org
    ```
1. 起動スクリプトを差し替え
    ```bash
    $ cp gas_all_start.sh.gps gas_all_start.sh
    ```
1. Wi-Fi無効化
    ```bash
    $ sudo rfkill block all
    ```
1. カメラ再起動（電源OFF→ON）  
  
## GASアプリへ戻す方法
1. TeraTarmを使いUART経由でカメラのコンソールに接続してログイン
1. 起動スクリプトを戻す
    ```bash
    $ cp gas_all_start.sh.org gas_all_start.sh
    ```
1. Wi-Fiを有効化
    ```bash
    $ sudo rfkill unblock 0
    ```
1. ホワイトリスト有効化  
    ※失敗した場合にカメラが起動しなくなる可能性があるので通常は実行しない
    ```bash
    $ ./WhiteListSwitching.sh -e
    ```
1. カメラ再起動
  