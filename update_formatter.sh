#!/bin/zsh
# Because Raspberry Pi is not usually connected to internet,
# this script pulls latest git and copies it to the Pi for testing.

# git pull
# scp -r ../foxsi-4matter foxsi@raspberrypi.local:/home/foxsi/foxsi-4matter
rsync -av --exclude=build --exclude=bin ../foxsi-4matter foxsi@raspberrypi.local:/home/foxsi/
echo "all done!"
