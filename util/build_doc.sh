#!/bin/zsh

# to be run from project root, foxsi-4matter/

echo "running doxygen..."
doxygen
echo "running sphinx..."
source doc/env/bin/activate
cd doc/breathe
sphinx-build -M html source build
sphinx-build -M latexpdf source build
echo "opening docs..."
open build/html/index.html
cd ../..