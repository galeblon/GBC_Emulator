~/EmulationStation/emulationstation.sh --no-exit &
xrandr --output HDMI-1 --mode 320x240
xrandr --output HDMI-2 --same-as HDMI-1 --scale-from 320x240
xrandr --output HDMI-3 --same-as HDMI-1 --scale-from 320x240
exec openbox-session
