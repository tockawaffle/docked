<div align="center">

# Docked

[![wakatime](https://wakatime.com/badge/user/e0979afa-f854-452d-b8a8-56f9d69eaa3b/project/fcee6354-8017-4563-ba53-8bc3f3a0d023.svg)](https://wakatime.com/badge/user/e0979afa-f854-452d-b8a8-56f9d69eaa3b/project/fcee6354-8017-4563-ba53-8bc3f3a0d023)

Welcome to Docked - Your DIY Stream Deck Alternative! 🎮

This project turns the WaveShare ESP32-7-LCD into an affordable Stream Deck alternative. Perfect for streamers, content creators, and anyone looking for a customizable touch interface!

> **Note**: Currently in early development (alpha stage). While functional, you may encounter bugs or incomplete features.

> **Important**: This project is specifically designed for the [WaveShare ESP32-7-LCD](https://www.waveshare.com/esp32-s3-touch-lcd-7.htm) and has not been tested with other hardware. We currently don't plan to support other devices.

</div>

## 🚀 Getting Started

### What You'll Need

#### Hardware Requirements
- [WaveShare ESP32-7-LCD](https://www.waveshare.com/esp32-s3-touch-lcd-7.htm) board
- USB-C cable for connecting the board
- A Windows PC for development

#### Software Requirements
- Windows 10 or 11
- WSL 2 (Windows Subsystem for Linux) with Ubuntu
- Visual Studio Code
- Basic familiarity with using command line interfaces

Don't worry if you don't have everything set up yet - we'll guide you through each step! 

## 📥 Installation Guide

> Note: This guide might be incomplete. I'm sorry.

### Step 1: Setting Up WSL (Windows Subsystem for Linux)

1. Open PowerShell as Administrator (right-click on the Start menu and select "Windows PowerShell (Admin)")
2. Install WSL by running:
   ```powershell
   wsl --install
   ```
3. Restart your computer when prompted
4. After restart, a Ubuntu window will open automatically - create your username and password when prompted
   > Keep these credentials safe - you'll need them later!

For more detailed instructions, check [Microsoft's WSL installation guide](https://learn.microsoft.com/en-us/windows/wsl/install).

### Step 2: Installing Visual Studio Code

1. Download Visual Studio Code from the [official website](https://code.visualstudio.com/)
2. Run the installer and follow the installation wizard
3. During installation, make sure these boxes are checked:
   - "Add 'Open with Code' action to Windows Explorer file context menu"
   - "Add 'Open with Code' action to Windows Explorer directory context menu"
   - "Add to PATH"

After installation:
1. Open VSCode
2. When prompted about installing the "WSL Extension", click "Install"
   > If you don't see this prompt, press `Ctrl+Shift+X`, search for "Remote - WSL" and install it
3. Wait for the installation to complete and click "Reload" if prompted

### Step 3: Installing ESP-IDF

The ESP-IDF (Espressif IoT Development Framework) is essential for building this project.

1. Open VSCode
2. Press `Ctrl+Shift+X` to open the Extensions panel
3. Search for "ESP-IDF" and install "ESP-IDF Extension"
4. Once installed, in VSCode:
   - Press `Ctrl+Shift+P`
   - Type "ESP-IDF: Configure ESP-IDF extension"
   - Select "Express"
   > This will start the express installation of ESP-IDF. It may take several minutes to complete. Do not stop the installation process.
5. Wait for the installation to finish
   > You'll see a notification when it's done
   > If you get an error, please go to the Discord and share it with us.
6. Close VSCode completely (this is important!)

### Step 4: Installing Required Packages

1. Open PowerShell and enter WSL by typing:
   ```powershell
   bash
   ```
   > You should see your username change to what you set up in Step 1

2. Install all required packages by copying and pasting this command:
   ```bash
   sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
   ```
   > When prompted for your password, type it (you won't see the characters as you type - this is normal!)

3. Verify Python installation:
   ```bash
   python3 --version
   ```
   You should see something like `Python 3.x.x`

4. If you have Python 2 installed and want to remove it (recommended):
   ```bash
   sudo apt-get remove python2.7
   sudo apt-get autoremove
   sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 1
   ```

### Step 5: Getting the Project Files

1. In your WSL terminal, run these commands:
   ```bash
   cd ~                   # Goes to your home directory
   git clone https://git.sipher.space/decksterity/docked.git
   cd docked             # Moves into the project directory
   code .                # Opens VSCode in this directory
   ```

2. When VSCode opens, you'll see a popup asking to install recommended extensions - click "Install All"
   > These extensions are essential for development. They include:
   > - ESP-IDF Tools for building and flashing
   > - C/C++ tools for code editing
   > - CMake tools for project configuration
   > - WSL remote development support

### Step 6: Connecting Your Device

This part requires setting up USBIPD to connect your ESP32 board to WSL. Let's do this step by step:

1. First, install USBIPD by following [Microsoft's guide](https://learn.microsoft.com/en-us/windows/wsl/connect-usb)

2. Once installed, connect your ESP32 board to your computer

3. Open a new PowerShell window as Administrator and run:
   ```powershell
   usbipd list
   ```
   > Look for your device in the list. It should show up as "Silicon Labs CP210x USB to UART Bridge" or similar

4. Note the BUSID (it looks like X-Y, for example "3-2") and run:
   ```powershell
   usbipd bind --busid X-Y
   usbipd attach --wsl --busid X-Y
   ```
   > Replace X-Y with your device's BUSID from the previous step

5. In your WSL terminal, run:
   ```bash
   ls /dev/ttyUSB* /dev/ttyACM*
   ```
   > This will show available serial ports. Note which one appears (usually /dev/ttyACM0 or /dev/ttyUSB0)

6. Set the permissions for the port:
   ```bash
   sudo chmod 666 /dev/ttyACM0  # Replace ttyACM0 with your port from step 5
   ```

### Step 7: Building and Flashing

1. In VSCode, press `Ctrl+Shift+P` and type "ESP-IDF: Open ESP-IDF Terminal"
> If nothing happens, you might need to enable the extension on WSL. Simply go to the "Extensions" tab and enable it.

2. You might want to change some things on "dependencies.lock" file. You might need to change "/home/tocka" to "/home/WSL-USERNAME", if you don't, you might get an error on the next step or even previous steps.

3. In the ESP-IDF terminal that opens, run:
   ```bash
   idf.py -p /dev/ttyACM0 flash  # Replace ttyACM0 with your port
   ```
   > This command will build the project and flash it to your device. The first build might take several minutes!

3. Wait for the process to complete. You'll see a success message when it's done.

## 🤝 Need Help?

If you run into any issues:
1. Check if you missed any steps in this guide
2. Join our [Discord community](https://discord.gg/)
3. Open an issue on our [repository](https://git.sipher.space/decksterity/docked)

---

## 📝 Repository Information

The primary repository is hosted on [Gitea](https://git.sipher.space/decksterity/docked). The GitHub repository serves as a mirror.

---

## 📜 License

This project is licensed under [AGPL v3](./LICENSE), with a special exception for logo assets:

### Logo and Assets License

The logo and assets in `components/ui/components/assets/logos` are licensed under [CC BY-NC-ND 4.0](./components/ui/components/assets/logos/LICENSE).

#### What You Can Do:
- Use the default logo as-is with the project
- Replace it with your own original design
- Remove it completely (but keep the folder structure)

#### What You Cannot Do:
- Modify the default logo
- Share modified versions
- Use the default logo commercially

Need special permissions? Contact [tocka@tockanest.com](mailto:tocka@tockanest.com)

For more details, see:
- [LICENSE.summary.md](./components/ui/components/assets/logos/LICENSE.summary.md) for a quick overview
- [LICENSE](./components/ui/components/assets/logos/LICENSE) for the full terms