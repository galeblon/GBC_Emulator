# Raspberry Pi installation

1. Flash Raspberry Pi OS Lite to SD card (tested on 2020-08-20-raspios-buster-armhf-lite.img)
2. Boot up Raspberry Pi and log in (`pi/raspberry`)
3. Run `sudo raspi-config` and configure:
	1. Localisation -> WLAN Country (if WLAN needed)
	3. Network -> WLAN (if WLAN needed)
	2. Localisation -> Keyboard
	4. Boot -> Desktop/CLI -> Console Autologin
	5. Reboot after finishing is not needed
4. Start sshd with `sudo systemctl start ssh` and check ip address (`ip a`)
5. From development setup copy `gbc.zip` to Raspberry Pi using:
	- `scp gbc.zip pi@<ip raspberry pi>:/home/pi`
	- password will be needed: `raspberry`
6. Run:
	- `unzip gbc.zip`
	- `cd gbc/deploy`
	- `sudo ./deploy.sh`
7. If everything completes without errors run `reboot`
8. After reboot `EmulationStation` should start by itself.
	- If no ROM medium was insterted on reboot, EmulationStation will display a message and not run. In this case insert a USB drive with roms and reboot Raspberry Pi (`Enter` -> right click -> `Terminal emulator` -> `reboot`

__At the moment the setup script is unable to rollback and if anything breaks YOU'RE ON YOUR OWN__ (although it is fairly simple, see `deploy.sh`)
