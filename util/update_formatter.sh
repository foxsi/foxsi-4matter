#!/bin/zsh
# Because Raspberry Pi is not usually connected to internet,
# this script pulls latest git and copies it to the Pi for testing.

# git pull
# scp -r ../foxsi-4matter foxsi@raspberrypi.local:/home/foxsi/foxsi-4matter
# rsync -av --exclude=build --exclude=bin ../foxsi-4matter foxsi@raspberrypi.local:/home/foxsi/
rsync -av --exclude=build --exclude=bin --exclude=doc ../foxsi-4matter foxsi@192.168.1.8:/home/foxsi/tvac
echo "all done!"
