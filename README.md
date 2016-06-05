# stm32-u2f
A Universal 2nd Factor (U2F) USB token using STM32 MCUs.

# Overview
The FIDO U2F protocol enables a strong cryptographic 2nd factor option for end user security. End users carry a single U2F device which works with any relying party supporting the protocol.

U2F security keys can currently be used with Google accounts as a method for two-step verification and is supported by Google Chrome since version 38. U2F security keys can also be used as an additional method for two-step verification for Dropbox and as of October 1, 2015, for GitHub.

Learn more about U2F on [https://fidoalliance.org/](https://fidoalliance.org/)

# How to build the code
The project was developed using the fabulus [visualgdb](http://www.visualgdb.com/) under Windows.
VisualGDB is a paid product, and it uses Visual Studio (i.e only supports Windows).
However, VisualGDB uses the opensource GNU toolchain internally, and also generates a Makefile - so it should be fairly easy to compile the code on any UNIX environment, after installing [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) toolchain.

Open the sulution using VisualGDB and build the project. Or drop to the command line and run make.

# Supported Hardware

Currently the project runs on the [stm32f4discovery](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/stm32f4discovery.html) board.
This board is available for [purchase](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/stm32f4discovery.html#samplebuy-scroll) directly from ST micro for less than 20$.

Based on the STM32F407VGT6 MCU, the board includes an ST-LINK/V2 or ST-LINK/V2-A embedded debug tool, two ST MEMS digital accelerometers, a digital microphone, one audio DAC with integrated class D speaker driver, LEDs and push buttons and an USB OTG micro-AB connector.

Key Features:

  * STM32F407VGT6 microcontroller featuring 32-bit ARM Cortex® -M4 with FPU core, 1-Mbyte Flash memory, 192-Kbyte RAM in an LQFP100 package. This model also has a **true random number generator (RNG)**.
  * On-board ST-LINK/V2 debugging port
  * Board power supply: through USB bus or from an external 5 V supply voltage
  * Eight LEDs
  * Two push buttons (user and reset)
  * USB OTG FS with micro-AB connector

This boards is very good for development because it has an onboard debugger. Besides memory and processing limits, and the availability of the RNG - nothing prevents from porting the code to other STM32 MCUs.
The intention is to support inexpensive STM32 boards like the [red pill and the blue pill](http://www.stm32duino.com/viewtopic.php?t=117) which could be purchased for 3$ on eBay.

# Debugging

  * Single stepping under the debugger - VisualGDB makes it very easy inside visual studio - just select Debug on the menu. VisualGDB will compile the code, flash the board, and allow single stepping and breakpoints.
    VisualGDB uses the [openocd](http://openocd.org/) (Open On-Chip Debugger) internally - so users interested in other environments can setup and use openocd directly on any supported host platform.
  * UART printing - 
    * You will need a separate USB to TTL UART adapter, those are plenty available from eBay under 5$, e.g [CP2102](http://www.ebay.com/itm/CP2102-USB-2-0-to-TTL-UART-Module-6Pin-Serial-Converter-STC-Replace-FT232-Module-/381374541932?hash=item58cbb1b06c:g:BxQAAOSwQiRUm-ND).
    * Connect the UART adapter pins to the discovery as follows:
      * PIN PC10 (TX) --> UART RX
      * PIN PC11 (RX) --> UART TX
      * GND --> UART GND
      * !! Do not connect the 3v or 5v pin from the UART to the discovery board !!
    * Open your favorite serial terminal app (e.g putty on Windows) and open the UART COM port, and select 38400 baud rate.
    * You will see UART debug prints displayed on the console.
    
# Keys
At the moment, all private keys (AES key, attestation private key and public cert) are all hard coded in keys.h file.
This is obviously insecure and incomplete. The intention is to implement a vendor private API (and tools) to help initialize a token - generate the AES private key inside the token, and also allow an "OEM" to set an existing attestation key pair onto the device.

# Entropy
Luckily the STM32F4 has a true random number generator - which is used to seed any also requiring random numbers.
For MCUs having no RNG, the ADC can be used as a source for entropy.

# Crypto
* The [MBED tls](https://tls.mbed.org/) opensource library is used for all crypto operations.
* A private AES key is used inside the token to export encrypted key handles.

# Issues
See [open issues](https://github.com/avivgr/stm32-u2f/issues) list.

# Status
The token currently successfully registers and authenticates against https://fidodemo.strongauth.com/fidorestapp/action
And also was successfully used for Google Account using chrome.

Patches are welcome!

