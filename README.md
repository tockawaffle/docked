# Embedded Decksterity

## About 📖

Decksterity is all about DIY, it's a cheap, good-enough option for expensive Stream Decks. This is highly alpha, not even that, so be careful while using it.

> **Note**: This was designed for [WaveShare's ESP32-7-LCD](https://www.waveshare.com/esp32-s3-touch-lcd-7.htm), have not and will not test on other screens or boards. Note that this is a custom board made by WaveShare.

## Usage Guide:

There is no easy way of doing this, you will need a lock of packages that might not be listed at all here, I wil eventually list all of them, but be careful.

You have to build it yourself and flash it yourself, there is no app that would flash it for you.

For flashing, you must know a bit of coding, else you'll struggle a lot.
I will, eventually, add a beginners guide to make this more accessible for non-technical users.

<details>
<summary>
Build Configuration
</summary>

#### Please, note that this project was compiled first on WSL with Ubuntu 24.02.

```sh
# Needs to be admin
PS > usbipd list # Lists the USB ports currently plugged on the computer.
PS > usbipd bind --busid PORT-PORT # Binds the ports, ex: usbipd bind --busid 5-3
PS > usbipd attach --wsl --busid # Attaches the ports to WSL, ex: usbipd attach --wsl --busid 5-3
```

Then

```bash
$ sudo chmod 666 /dev/PORT # PORT needs to be the port that it was connected as, ex: sudo chmod 666 /dev/ttyACM0

# Now you open the IDF terminal and executes the following command inside this folder:
$ idf.py -p /dev/PORT flash # This will build AND flash the current plugged device.
```
</details>

---

You're ready to go!