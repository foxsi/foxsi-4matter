#!/bin/zsh
# copies files in remote formatter:/home/foxsi/foxsi-4matter/logs/* into local log/ folder, under log/formatter/

mkdir log/formatter

rsync -av  foxsi@192.168.1.8:/home/foxsi/foxsi-4matter/log/ ../foxsi-4matter/log/formatter/

echo "copied log files"
