# sawMaxonEPOS

This SAW package contains the `mtsMaxonEPOS` component, which interfaces with Maxon controllers using the EPOS Command Library over USB. To use this package, download and install the EPOS Command Library from:
  - Linux: [Version 6.8.1.0](https://www.maxongroup.com/medias/sys_master/root/8994700394526/EPOS-Linux-Library-En.zip)
  - Windows: [Version 6.8.1.0](https://www.maxongroup.com/medias/sys_master/root/8994700197918/EPOS-Windows-DLL-En.zip)

On Linux, it is necessary to install the library using the `install.sh` provided by Maxon, which creates the necessary soft-links in `/usr/lib`.

The component is designed to be generic and is configured via a JSON file.
See the [README](./core/share/README.md) in the `share` directory for details and an example.

This component supports controlling multiple controllers simultaneously. The computer communicates with the master controller via USB, and additional controllers are daisy-chained over CAN. Please refer to the hardware manual for wiring details.

**Control frequency:** TBD (to be determined through testing).

> **Note:**  
> Each EPOS controller has onboard EEPROM that stores the calibration parameters for its connected motor.  
> Calibration **must** be performed before first use. To calibrate, use Maxon’s EPOS Studio. Once calibration is complete and the parameters are downloaded to the controller, no further calibration is required.

Most of the source code lives in the `core` subdirectory. A console test program is also provided as an example of usage.