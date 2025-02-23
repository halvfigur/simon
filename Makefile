build: simon.ino game.ino
	arduino-cli compile

upload: build
	arduino-cli  upload -b arduino:avr:uno -p /dev/ttyACM0 --verbose --verify

.PHONY: monitor
monitor:
	arduino-cli monitor -p /dev/ttyACM0 -b arduino:avr:uno
