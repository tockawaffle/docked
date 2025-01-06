# ESP32-UI

UI/UX for the ESP32-LCD-7 base

Please, build before doing anything. It requires some things from the build.

For development, you can use the default build command that's on VSC ESP-IDF extension.

Commands used to build:
```sh
# Needs to be admin
PS > usbipd list # Lists the USB ports currently plugged on the computer.
PS > usbipd bind --busid PORT-PORT # Binds the ports, ex: usbipd bind --busid 5-3
PS > usbipd attach --wsl --busid # Attaches the ports to WSL, ex: usbipd attach --wsl --busid 5-3
```

Then

```bash
# PORT needs to be the port that it was connected as, ex: sudo chmod 666 /dev/ttyACM0
$ sudo chmod 666 /dev/PORT 

# Now you open the IDF terminal and executes the following command inside this folder:
$ idf.py -p /dev/PORT flash # This will build AND flash the current plugged device.
```

You're ready to go!