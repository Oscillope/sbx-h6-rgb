# Soundblaster X H6 RGB

This repository tracks my attempt to reverse-engineer the USB protocol to set the color of the light ring on the SoundblasterX H6 headset to allow changing the color from Linux.

The audio part of the headset works using the default ALSA USB soundcard drivers, but there is a separate USB HCI interface for the LEDs (and presumably the volume control knob, haven't gotten to that yet).

Right now I just wanted to track the very (VEEEERRRRYYY) basic app I wrote which allows you to set the LED color. I've also uploaded an example udev rule that sets the LEDs to purple whenever the headset is plugged in. Drop it in `/etc/udev/rules.d` if you want to try it out.

The `sbx-h6-led.service` file is a systemd unit file that will set the headset's color to the factory default's  red (`ff0000`) and the brightness to `255` at startup. It will set the color to red (`ff0000`) and the brightness to `0` on shutdown to make sure the headset's LEDs are off when you turn off the machine. These values, can of course be changed to your liking. This was created mostly to deal with the fact that by default when you turn your PC off, the LEDs on the headset will stay on. In order to use this service file, just copy it to `/etc/systemd/system/` and enable it with the command `systemctl enable sbx-h6-led.service` (root privileges required). Also note that the value `041e:3255` is 'hard-coded' in this systemd file. I assume this would be the same for all "SoundBlasterX H6" devices, but YMMV. You can verify if your headset has the same ID with the command `lsusb |grep "BlasterX"`.

I've also uploaded a small packet capture to illustrate the LED setting routine from the original windows driver. I'll upload more as I test out more features.
