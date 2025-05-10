#!/bin/sh

export LD_LIBRARY_PATH="/mnt/SDCARD/ROMS/examples:/customer/lib:/lib:/lib:/config/lib:/mnt/SDCARD/miyoo/lib:/mnt/SDCARD/.tmp_update/lib:/mnt/SDCARD/.tmp_update/lib/parasyte"

PID=$(pidof MainUI)
echo $PID
kill -STOP $PID
./SDL2W_EXAMPLE
kill -CONT $PID