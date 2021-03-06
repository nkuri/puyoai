puyoai
======

ぷよぷよのAIを書くためのフレームワークです。

## ビルド方法

現状、Mac OS X(10.10 Yosemite) と Ubuntu Linux 14.04 でビルドできることを確認しています。

古めの Linux (Ubuntu 12.04 など)では、gccやclangが古いので、もしかしたらコンパイルが通らない
かもしれません。また、SDL 2.0がaptにありませんので、どこかから取ってくるか、自分でビルドしなければなりません。

基本的に-std=c++11で書かれています。

### 必要なライブラリ

次のライブラリが必要です。

* google-gflags
* google-glog

GUI を付与したい場合、さらに

* SDL 2.0
* SDL_ttf (2.0)
* SDL_image (2.0)

ビデオキャプチャーをしたい場合、さらに

* gcrypt
* lib-usb1.0

### make のしかた

outディレクトリを掘って、そこでビルドするのがオススメです。

    $ mkdir -p out/Release; cd out/Release
    $ cmake ../../src
    $ make -j8
    $ make test

* gflags と glog が cmake に発見されなかった場合、cmake が成功しません。
* SDL と SDL_ttf がない場合、cmakeは成功しますが、GUIがつきません。
* capture/ ディレクトリについては、 capture/README を参照してください。

### mac の場合

Macの場合、homebrewを使うと楽です。Xcode、コマンドラインツール、homebrewをインストールしてください。デフォルトで/usr/local/includeなどを見ないようになっている可能性がありますが、xcode-select --installを叩いておくと解決します。

次のコマンドで必要なものが入るとおもいます。

    $ brew install pkg-config
    $ brew install gflags glog sdl2 SDL2_ttf SDL2_image ffmpeg libusb libgcrypt

## 実行

    $ cd out/Release
    $ ./duel/duel ./cpu/sample/sample ./cpu/sample/sample

sample は、消せるところがあれば消し、そうでなければ左に積むだけのアルゴリズムで動いています。

## ディレクトリの説明

* src/ 主なプログラムのソース
* arduino/ Wii実機と接続する際に使う、arduino関連
* data/    フォントとか画像とか
* testdata/ キャプチャー用のテストデータ
* tools/ いろいろなツール類
* deprecated/ 過去のソース。今ほどモジュールに分かれていない。

### そのほか

雑な細かい説明がここに書いてあります。

http://d.hatena.ne.jp/shinichiro_h/20130203

AI のプロトコル仕様について、とても雑で不完全なドキュメントはここにあります。

https://docs.google.com/document/d/1cTIJgRHCBhmxXVtBb45Jm3yAQVaogfVN3oayi-FrA74/edit

適当な情報を wiki に足していく予定です…

https://github.com/puyoai/puyoai/wiki
