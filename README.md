# melotts.axcl
MeloTTS demo on Axera Card

## 编译

- 支持在 x86_64、aarch64 平台下本地编译
- 支持在 x86_64 下交叉编译生成 aarch64 环境中可运行的程序

x86_64
```
./build.sh
```

aarch64
```
./build_aarch64.sh
```

交叉编译
```
./cross_compile.sh
```

## 下载模型
```
./download_models.sh
```

## 运行
在 `melotts.axcl` 项目根目录下运行

```
(base) axera@raspberrypi:~/melotts.axcl $ ./install/melotts
encoder: ./models/encoder.onnx
decoder: ./models/decoder.axmodel
lexicon: ./models/lexicon.txt
token: ./models/tokens.txt
sentence: 爱芯元智半导体股份有限公司，致力于打造世界领先的人工智能感知与边缘计算芯片。服务智慧城市、智能驾驶、机器人的海量普惠的应用
wav: output.wav
speed: 0.800000
sample_rate: 44100
Load encoder
Load decoder model
Encoder run take 191.25ms
decoder slice num: 9
Decode slice(1/9) take 39.90ms
Decode slice(2/9) take 39.66ms
Decode slice(3/9) take 39.98ms
Decode slice(4/9) take 39.57ms
Decode slice(5/9) take 40.28ms
Decode slice(6/9) take 39.68ms
Decode slice(7/9) take 39.59ms
Decode slice(8/9) take 39.58ms
Decode slice(9/9) take 41.11ms
Saved audio to output.wav

```
更多运行参数:  
```
(base) axera@raspberrypi:~/melotts.axcl $ ./install/melotts -h
undefined short option: -h
usage: ./install/melotts [options] ...
options:
  -e, --encoder        encoder onnx (string [=./models/encoder.onnx])
  -d, --decoder        decoder axmodel (string [=./models/decoder.axmodel])
  -l, --lexicon        lexicon.txt (string [=./models/lexicon.txt])
  -t, --token          tokens.txt (string [=./models/tokens.txt])
      --g              g.bin (string [=./models/g.bin])
  -s, --sentence       input sentence (string [=爱芯元智半导体股份有限公司，致力于打造世界领先的人工智能感知与边缘计算芯片。服务智慧城市、智能驾驶、机器人的海量普惠的应用])
  -w, --wav            wav file (string [=output.wav])
      --speed          speak speed (float [=0.8])
      --sample_rate    sample rate (int [=44100])
  -?, --help           print this message
```