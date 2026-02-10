#!/bin/sh
mkdir dist
cd src
python setup.py build_ext --inplace
cd ..
cp src/torque*.cpython* -v dist/
cp src/pycontrol/*.py src/pycontrol/*.html -v dist/