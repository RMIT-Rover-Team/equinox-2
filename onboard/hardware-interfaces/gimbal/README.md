# Gimbal Hardware Interface

TODO: add USB bandwidth monitor (is this needed?)

Make sure you have the. `usbmon` kernel module enabled

Enable with:
```
$ mount -t debugfs none_debugs /sys/kernel/debug
$ modprobe usbmon
```