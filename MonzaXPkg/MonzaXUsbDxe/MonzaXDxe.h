/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

**/

#ifndef _MONZAX_DXE_H_
#define _MONZAX_DXE_H_


#include <Uefi.h>

#include <Protocol/DevicePath.h>
#include <Protocol/UsbIo.h>
#include <Protocol/MonzaXIo.h>

#include <Library/ReportStatusCodeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/MonzaXLib.h>

#include "SiliconLabCP2112.h"

#define MONZAX_DEV_SIGNATURE SIGNATURE_32 ('m', 'z', 'x', 'u')

typedef struct {
  UINT16          IdVendor;
  UINT16          IdProduct;
  UINT8           DeviceClass;
  UINT8           DeviceSubClass;
  UINT8           DeviceProtocol;

  UINT8           InterfaceClass;
  UINT8           InterfaceSubClass;
  UINT8           InterfaceProtocol;
} MONZAX_USB_INFO;

typedef struct {
  UINTN                         Signature;

  EFI_HANDLE                    ControllerHandle;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;

  EFI_USB_IO_PROTOCOL           *UsbIo;

  MONZAX_IO_PROTOCOL            MonzaXIo;

  UINT8                         MonzaxI2cDeviceId;
  MONZAX_CHIP_MODEL_TYPE        ChipModelType;

  EFI_USB_DEVICE_DESCRIPTOR     DeviceDescriptor;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   InEndpointDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   OutEndpointDescriptor;

  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
} MONZAX_DEV;

#define MONZAX_DEV_FROM_MONZAX_IO_PROTOCOL(a) \
    CR(a, MONZAX_DEV, MonzaXIo, MONZAX_DEV_SIGNATURE)

extern EFI_DRIVER_BINDING_PROTOCOL  gMonzaXDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gMonzaXComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gMonzaXComponentName2;

/**
  Retrieves a Unicode string that is the user readable name of the driver.

  This function retrieves the user readable name of a driver in the form of a
  Unicode string. If the driver specified by This has a user readable name in
  the language specified by Language, then a pointer to the driver name is
  returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
  by This does not support the language specified by Language,
  then EFI_UNSUPPORTED is returned.

  @param  This                  A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.
  @param  Language              A pointer to a Null-terminated ASCII string
                                array indicating the language. This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified
                                in RFC 4646 or ISO 639-2 language code format.
  @param  DriverName            A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                driver specified by This in the language
                                specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by
                                This and the language specified by Language was
                                returned in DriverName.
  @retval EFI_INVALID_PARAMETER Language is NULL.
  @retval EFI_INVALID_PARAMETER DriverName is NULL.
  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
MonzaXComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );

/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by a driver.

  This function retrieves the user readable name of the controller specified by
  ControllerHandle and ChildHandle in the form of a Unicode string. If the
  driver specified by This has a user readable name in the language specified by
  Language, then a pointer to the controller name is returned in ControllerName,
  and EFI_SUCCESS is returned.  If the driver specified by This is not currently
  managing the controller specified by ControllerHandle and ChildHandle,
  then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
  support the language specified by Language, then EFI_UNSUPPORTED is returned.

  @param  This                  A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.
  @param  ControllerHandle      The handle of a controller that the driver
                                specified by This is managing.  This handle
                                specifies the controller whose name is to be
                                returned.
  @param  ChildHandle           The handle of the child controller to retrieve
                                the name of.  This is an optional parameter that
                                may be NULL.  It will be NULL for device
                                drivers.  It will also be NULL for a bus drivers
                                that wish to retrieve the name of the bus
                                controller.  It will not be NULL for a bus
                                driver that wishes to retrieve the name of a
                                child controller.
  @param  Language              A pointer to a Null-terminated ASCII string
                                array indicating the language.  This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified in
                                RFC 4646 or ISO 639-2 language code format.
  @param  ControllerName        A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                controller specified by ControllerHandle and
                                ChildHandle in the language specified by
                                Language from the point of view of the driver
                                specified by This.

  @retval EFI_SUCCESS           The Unicode string for the user readable name in
                                the language specified by Language for the
                                driver specified by This was returned in
                                DriverName.
  @retval EFI_INVALID_PARAMETER ControllerHandle is NULL.
  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid
                                EFI_HANDLE.
  @retval EFI_INVALID_PARAMETER Language is NULL.
  @retval EFI_INVALID_PARAMETER ControllerName is NULL.
  @retval EFI_UNSUPPORTED       The driver specified by This is not currently
                                managing the controller specified by
                                ControllerHandle and ChildHandle.
  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
MonzaXComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  );

/**
  Check whether USB MonzaX driver supports this device.

  @param  This                   The USB MonzaX driver binding protocol.
  @param  Controller             The controller handle to check.
  @param  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The driver supports this controller.
  @retval other                  This device isn't supported.

**/
EFI_STATUS
EFIAPI
MonzaXDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

/**
  Starts the MonzaX device with this driver.

  This function consumes USB I/O Portocol, intializes USB MonzaX device,
  installs MonzaX IO Protocol.

  @param  This                  The USB MonzaX driver binding instance.
  @param  Controller            Handle of device to bind driver to.
  @param  RemainingDevicePath   Optional parameter use to pick a specific child
                                device to start.

  @retval EFI_SUCCESS           This driver supports this device.
  @retval EFI_UNSUPPORTED       This driver does not support this device.
  @retval EFI_DEVICE_ERROR      This driver cannot be started due to device Error.
  @retval EFI_OUT_OF_RESOURCES  Can't allocate memory resources.
  @retval EFI_ALREADY_STARTED   This driver has been started.

**/
EFI_STATUS
EFIAPI
MonzaXDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

/**
  Stop the USB MonzaX device handled by this driver.

  @param  This                   The USB MonzaX driver binding protocol.
  @param  Controller             The controller to release.
  @param  NumberOfChildren       The number of handles in ChildHandleBuffer.
  @param  ChildHandleBuffer      The array of child handle.

  @retval EFI_SUCCESS            The device was stopped.
  @retval EFI_UNSUPPORTED        MonzaX IO Protocol is not installed on Controller.
  @retval Others                 Fail to uninstall protocols attached on the device.

**/
EFI_STATUS
EFIAPI
MonzaXDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  );

/**
  Return MonzaX USB information.

  @param UsbIo  USB IO instance

  @return MonzaX USB information.
  @retval NULL This USB is not MonzaX USB device.
**/
MONZAX_USB_INFO *
IsMonzaXUsb (
  IN EFI_USB_IO_PROTOCOL  *UsbIo
  );

/**

  Get MonzaX chip information.

  @param This     Pointer to the MONZAX_IO_PROTOCOL instance.
  @param Info     The buffer to hold MonzaX chip information.

  @retval EFI_SUCCESS            The MonzaX information structure is returned.
  @retval EFI_INVALID_PARAMETER  Info is NULL.
  @retval EFI_BUFFER_TOO_SMALL   The Info buffer is too small to hold the full data.

**/
EFI_STATUS
EFIAPI
MonzaXIoGetInfo (
  IN  MONZAX_IO_PROTOCOL             *This,
  IN OUT MONZAX_INFO                 *Info
  );

/**

  Set MonzaX chip information.

  @param This     Pointer to the MONZAX_IO_PROTOCOL instance.
  @param Info     The buffer to hold MonzaX chip information.

  @retval EFI_SUCCESS            The MonzaX information is set.
  @retval EFI_INVALID_PARAMETER  Info is NULL.

**/
EFI_STATUS
EFIAPI
MonzaXIoSetInfo (
  IN  MONZAX_IO_PROTOCOL             *This,
  IN  MONZAX_INFO                    *Info
  );

/**

  Read data from MonzaX chip.

  @param This       Pointer to the MONZAX_IO_PROTOCOL instance.
  @param Address    The device address of MonzaX chip on where the data is read from.
  @param Data       A pointer to the buffer of data that will be read from MonzaX device.
  @param DataLenght On input, indicates the size, in bytes, of the data buffer specified by Data.
                    On output, indicates the amount of data actually transferred.

  @retval EFI_SUCCESS            The data is read successfully.
  @retval EFI_INVALID_PARAMETER  DataLength is NULL.
  @retval EFI_INVALID_PARAMETER  *DataLength is 0.
  @retval EFI_INVALID_PARAMETER  Data is NULL.
  @retval EFI_DEVICE_ERROR       Read data fail due to device error.

**/
EFI_STATUS
EFIAPI
MonzaXIoRead (
  IN  MONZAX_IO_PROTOCOL             *This,
  IN  UINT16                         Address,
  OUT UINT8                          *Data,
  IN OUT UINTN                       *DataLen
  );

/**

  Write data to MonzaX chip.

  @param This       Pointer to the MONZAX_IO_PROTOCOL instance.
  @param Address    The device address of MonzaX chip on where the data is written to.
  @param Data       A pointer to the buffer of data that will be written to MonzaX device.
  @param DataLength On input, indicates the size, in bytes, of the data buffer specified by Data.
                    On output, indicates the amount of data actually transferred.

  @retval EFI_SUCCESS            The data is written successfully.
  @retval EFI_INVALID_PARAMETER  DataLength is NULL.
  @retval EFI_INVALID_PARAMETER  *DataLength is 0.
  @retval EFI_INVALID_PARAMETER  Data is NULL.
  @retval EFI_DEVICE_ERROR       Write data fail due to device error.

**/
EFI_STATUS
EFIAPI
MonzaXIoWrite (
  IN  MONZAX_IO_PROTOCOL             *This,
  IN  UINT16                         Address,
  IN  UINT8                          *Data,
  IN OUT UINTN                       *DataLen
  );

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  );

#endif
