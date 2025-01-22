

<div align="center">

# Docked

[![wakatime](https://wakatime.com/badge/user/e0979afa-f854-452d-b8a8-56f9d69eaa3b/project/fcee6354-8017-4563-ba53-8bc3f3a0d023.svg)](https://wakatime.com/badge/user/e0979afa-f854-452d-b8a8-56f9d69eaa3b/project/fcee6354-8017-4563-ba53-8bc3f3a0d023)

Docked is all about DIY, it's a cheap, good-enough option for expensive Stream Decks. This is highly alpha, not even that, so be careful while using it.

> **Note**: This was designed for [WaveShare's ESP32-7-LCD](https://www.waveshare.com/esp32-s3-touch-lcd-7.htm), have not and will not test on other screens or boards. Note that this is a custom board made by WaveShare.

</div>

## Usage Guide:

There is no easy way of doing this, you will need a lot of packages that might not be listed at all here, I wil eventually list all of them, but be careful.

You have to build it yourself and flash it yourself, there is no app that would flash it for you.

For flashing, you must know a bit of coding, else you'll struggle a lot.
I will, eventually, add a beginners guide to make this more accessible for non-technical users.

<details>
<summary>
Build Configuration
</summary>

#### Please, note that this project was compiled first on WSL with Ubuntu 24.02.

```sh
# Currently this does not reflect the real building guide, this is more of a personal note for me to not forget what commands I run. I will add a proper build guide later.

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

## License

This project is licensed under the [AGPL v3](./LICENSE) license, which applies to all source code within the repository. However, please note the following exception:

### Logo and Asset Licensing
The logo and related assets located in the folder `components/ui/components/assets/logos` are protected under the **[CC BY-NC-ND 4.0](./components/ui/components/assets/logos/LICENSE)** license. 

#### Key Guidelines for Using the Logo and Assets:
- **Default Logo**: You may use the default logo **unchanged** with the project.
- **Replacing the Logo**: You may replace the logo with your own custom, entirely original design.
- **Removing the Logo**: You may remove the default logo from the project, but the folder must not be left empty as this will cause issues with the code.
- **Prohibited Actions**:
  - You may not modify the default logo.
  - You may not distribute modified versions of the default logo.
  - You may not use the default logo for commercial purposes.

If the logo is replaced or removed, you may remove the accompanying license files (`LICENSE` and `LICENSE.summary.md`) from the assets folder, as the **CC BY-NC-ND 4.0** license no longer applies.

#### Permissions Beyond This License
If you require permissions beyond the terms of this license (e.g., commercial use of the logo), please contact [tocka@tockanest.com](mailto:tocka@tockanest.com).

For more details, refer to:
- [LICENSE.summary.md](./components/ui/components/assets/logos/LICENSE.summary.md) for a quick overview
- [LICENSE](./components/ui/components/assets/logos/LICENSE) for the full legal text

---

You're ready to go!