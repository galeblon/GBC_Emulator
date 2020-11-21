import time
import os
import shutil

TIMEOUT = 10.0
SOURCE = r'/media/usb'
DESTINATION = r'/home/pi/roms'
EXTENSIONS = ('.gbc', '.gb', '.GBC', '.GB')

def copy_roms(ls):
	def is_rom(filename):
		return os.path.splitext(filename)[-1] in EXTENSIONS

	roms = list(filter(is_rom, ls))
	
	for rom in roms:
		shutil.copy(os.path.join(SOURCE, rom), DESTINATION)

def main():
	start = time.time()
	while time.time() - start < TIMEOUT:
		ls = os.listdir(SOURCE)
		
		if len(ls) > 0:
			copy_roms(ls)


if __name__ == "__main__":
	main()
