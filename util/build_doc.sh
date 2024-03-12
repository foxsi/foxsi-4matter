#!/bin/zsh

# to be run from project root, foxsi-4matter/

echo "running doxygen..."
doxygen
echo "running sphinx..."
source doc/env/bin/activate
sphinx-build -M html doc/breathe/source doc/breathe/build
echo "opening docs..."
open doc/breathe/build/html/index.html