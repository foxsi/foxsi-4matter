#!/bin/zsh
# Because Raspberry Pi is not usually connected to internet,
# this script updates the Pi with the current local changes.

# git pull
# scp -r ../foxsi-4matter foxsi@raspberrypi.local:/home/foxsi/foxsi-4matter
# rsync -av --exclude=build --exclude=bin ../foxsi-4matter foxsi@raspberrypi.local:/home/foxsi/
rsync -av --exclude=build --exclude=bin --exclude=doc --exclude=log --exclude=util/mock ../foxsi-4matter foxsi@192.168.1.8:/home/foxsi
echo "all done!"
