# Soundblaster X H6 RGB

This repository tracks my attempt to reverse-engineer the USB protocol to set the color of the light ring on the SoundblasterX H6 headset to allow changing the color from Linux.

The audio part of the headset works using the default ALSA USB soundcard drivers, but there is a separate USB HCI interface for the LEDs (and presumably the volume control knob, haven't gotten to that yet).

At some point I'll upload packet captures and stuff like that. Right now I just wanted to track the very (VEEEERRRRYYY) basic app I wrote which allows you to set the LED color. I've also uploaded an example udev rule that sets the LEDs to purple whenever the headset is plugged in. Drop it in `/etc/udev/rules.d` if you want to try it out.
