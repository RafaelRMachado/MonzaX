# This MonzaXPkg is the sample code to use MonzaX chip in UEFI environment.

## How to build
Just build it as normal EDKII package.
  0) Download EDKII from github https://github.com/tianocore/edk2
  1) Install Visual Studio 2015.
  2) type "edksetup.bat"
  3) type "build -p MonzaXPkg\MonzaXPkg.dsc -a IA32 -a X64 -t VS2015x86"

## Feature:
1) Software API:
   This package provides a set of UEFI API similar as Monza X software API. [MonzaApi]
   See MonzaXPkg\Include\Library\MonzaXLib.h

2) Chip:
   This package supports Monza X-2K Dura [MonzaX2K] and Monza X-8K Dura [MonzaX8K] chip.
   The support is integrated to MonzaXPkg\Library\UefiMonzaXLib, MonzaXPkg\MonzaXI2cDxe,
   and MonzaXPkg\MonzaXUsbDxe.

3) Interface:
   This package supports I2C interface [MonzaX2K][MonzaX8K] and USB interface using CP2112 [MonzaXUsb CP2112].
   The I2C interface is supported by MonzaXPkg\MonzaXI2cDxe.
   The USB interface is supported by MonzaXPkg\MonzaXUsbDxe.

## Known limitation
This code passes build only.
This code is NOT validated yet, we are still waiting for hardware.

## Reference:

[MonzaApi] Monza X Software API and Application Note
https://support.impinj.com/hc/en-us/articles/202756838-Monza-X-Software-API-and-Application-Note

[MonzaX2K] Monza X-2K Dura Datasheet
https://support.impinj.com/hc/en-us/articles/202756848-Monza-X-2K-Dura-Datasheet

[MonzaX8K] Monza X-8K Dura Datasheet
https://support.impinj.com/hc/en-us/articles/202756868-Monza-X-8K-Dura-Datasheet

[MonzaXUsb CP2112] CP2112 INTERFACE SPECIFICATION (Used by MonzaX RFID USB Device)
https://www.silabs.com/Support%20Documents/TechnicalDocs/AN495.pdf

