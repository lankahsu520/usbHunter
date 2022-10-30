# 1. Overview
usbHunter is an application that checks usb plugged and unplugged. Find which TTY device connected over USB.

You don't need to spend too much time on libusb.

# 2. Depend on

- [utilx9](https://github.com/lankahsu520/utilx9)
- [libusb](https://github.com/libusb/libusb)

# 3. Current Status



# 4. Build
   ```
$ make
   ```

# 5. Example or Usage
```bash
$ ./usbHunter -h
Usage: usbHunter
  -d, --debug       debug level
  -s, --srv         daemon mode
  -h, --help
Version: 0x01004000, d0d931f, 1667097888, lanka, 1667098158
Example:
  ./usbHunter -d2 -s

```

```bash
$ TTY_NAME=`usbHunter | grep "10C4:EA60" | cut -d";" -f 4`

```



# 6. License

usbHunter is under the New BSD License (BSD-3-Clause).


# 7. Documentation
Run an example and read it.
