#!/bin/bash
if [ ! -f models/decoder.axmodel ] || [ ! -f models/encoder.onnx ]; then
    wget -c https://github.com/ml-inory/melotts.axcl/releases/download/v1.0/models.tar.gz
    tar zxvf models.tar.gz
fi