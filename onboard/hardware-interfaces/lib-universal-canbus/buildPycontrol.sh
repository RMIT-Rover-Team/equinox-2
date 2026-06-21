#!/bin/sh
mkdir dist
cd libuniversalcan
python3 setup.py build_ext --inplace
cd ..
cp libuniversalcan/py*.cpython* -v dist/
