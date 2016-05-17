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

#include "MonzaXDxe.h"

EFI_DRIVER_BINDING_PROTOCOL gMonzaXDriverBinding = {
  MonzaXDriverBindingSupported,
  MonzaXDriverBindingStart,
  MonzaXDriverBindingStop,
  0xa,
  NULL,
  NULL
};

MONZAX_IO_PROTOCOL         mMonzaXIo = {
  MonzaXIoGetInfo,
  MonzaXIoSetInfo,
  MonzaXIoRead,
  MonzaXIoWrite
};

MONZAX_USB_INFO mMonzaXUsbInfo[] = {
  // Silicon Laboratories, Inc.
  {CP2112_VID, CP2112_PID, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00},
};

/**
  Entrypoint of USB MonzaX Driver.

  This function is the entrypoint of USB MonzaX Driver. It installs Driver Binding
  Protocols together with Component Name Protocols.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
MonzaXUsbDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gMonzaXDriverBinding,
             ImageHandle,
             &gMonzaXComponentName,
             &gMonzaXComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}


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
  )
{
  EFI_STATUS          Status;
  EFI_USB_IO_PROTOCOL *UsbIo;
  MONZAX_USB_INFO     *MonzaXInfo;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  (VOID **) &UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Use the USB I/O Protocol interface to check whether Controller is
  // a MonzaX device that can be managed by this driver.
  //
  Status = EFI_SUCCESS;
  MonzaXInfo = IsMonzaXUsb (UsbIo);
  if (MonzaXInfo == NULL) {
    Status = EFI_UNSUPPORTED;
  } else {
    DEBUG ((EFI_D_ERROR, "MonzaXDriverBindingSupported: Find!\n"));
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}


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
  )
{
  EFI_STATUS                  Status;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  MONZAX_DEV                  *MonzaXDevice;
  EFI_DEVICE_PATH             *DevicePath;
  UINT8                       EndpointNumber;
  EFI_USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;
  UINT8                       Index;
  BOOLEAN                     FoundIn;
  BOOLEAN                     FoundOut;
  EFI_TPL                     OldTpl;
  UINT8                       Result;

  DEBUG ((EFI_D_ERROR, "MonzaXDriverBindingStart: Enter\n"));

  MonzaXDevice = NULL;

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);
  //
  // Open USB I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  (VOID **) &UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit1;
  }

  //
  // Get the Device Path Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  MonzaXDevice = AllocateZeroPool (sizeof (MONZAX_DEV));
  ASSERT (MonzaXDevice != NULL);

  MonzaXDevice->Signature         = MONZAX_DEV_SIGNATURE;
  MonzaXDevice->UsbIo             = UsbIo;
  CopyMem (&MonzaXDevice->MonzaXIo, &mMonzaXIo, sizeof(mMonzaXIo));
  MonzaXDevice->DevicePath        = DevicePath;
  MonzaXDevice->ControllerHandle  = Controller;

  UsbIo->UsbGetDeviceDescriptor (
           UsbIo,
           &MonzaXDevice->DeviceDescriptor
           );

  //
  // Get interface & endpoint descriptor
  //
  UsbIo->UsbGetInterfaceDescriptor (
           UsbIo,
           &MonzaXDevice->InterfaceDescriptor
           );

  EndpointNumber = MonzaXDevice->InterfaceDescriptor.NumEndpoints;

  //
  // Traverse endpoints to find interrupt endpoint
  //
  FoundIn = FALSE;
  FoundOut = FALSE;
  for (Index = 0; Index < EndpointNumber; Index++) {
    UsbIo->UsbGetEndpointDescriptor (
             UsbIo,
             Index,
             &EndpointDescriptor
             );
    DEBUG ((EFI_D_INFO, "  Endpoint: 0x%x\n", Index));
    DEBUG ((EFI_D_INFO, "    Address:       0x%02x\n", EndpointDescriptor.EndpointAddress));
    DEBUG ((EFI_D_INFO, "    Attributes:    0x%02x\n", EndpointDescriptor.Attributes));
    DEBUG ((EFI_D_INFO, "    MaxPacketSize: 0x%04x\n", EndpointDescriptor.MaxPacketSize));
    DEBUG ((EFI_D_INFO, "    Interval:      0x%02x\n", EndpointDescriptor.Interval));
    if ((EndpointDescriptor.EndpointAddress & 0x80) == 0) {
      if ((EndpointDescriptor.Attributes & (BIT0 | BIT1)) == USB_ENDPOINT_INTERRUPT) {
        CopyMem(&MonzaXDevice->OutEndpointDescriptor, &EndpointDescriptor, sizeof(EndpointDescriptor));
        FoundOut = TRUE;
      }
    } else {
      if ((EndpointDescriptor.Attributes & (BIT0 | BIT1)) == USB_ENDPOINT_INTERRUPT) {
        CopyMem(&MonzaXDevice->InEndpointDescriptor, &EndpointDescriptor, sizeof(EndpointDescriptor));
        FoundIn = TRUE;
      }
    }
  }

  if ((!FoundIn) || (!FoundOut)) {
    //
    // No interrupt endpoint found, then return unsupported.
    //
    DEBUG ((EFI_D_INFO, "NO EndPoint found!\n"));
    Status = EFI_UNSUPPORTED;
    goto ErrorExit;
  }

  DEBUG ((EFI_D_INFO, "MonzaxChipTest\n"));
  // Hardcode
  MonzaXDevice->MonzaxI2cDeviceId = MONZAX_I2C_DEVICE_ID_DEFAULT;
  MonzaXDevice->ChipModelType = MonzaX8KDura;
  Result = MonzaxChipTest (&MonzaXDevice->MonzaXIo);
  if (Result == 0) {
    DEBUG ((EFI_D_INFO, "MonzaxChipTest - PASS!\n"));
  } else {
    DEBUG ((EFI_D_INFO, "MonzaxChipTest - FAIL!\n"));
    Status = EFI_UNSUPPORTED;
    goto ErrorExit;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gMonzaXIoProtocolGuid,
                  &MonzaXDevice->MonzaXIo,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Open For Child Device
  //

  MonzaXDevice->ControllerNameTable = NULL;
  AddUnicodeString2 (
    "eng",
    gMonzaXComponentName.SupportedLanguages,
    &MonzaXDevice->ControllerNameTable,
    L"Generic MonzaX USB",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gMonzaXComponentName2.SupportedLanguages,
    &MonzaXDevice->ControllerNameTable,
    L"Generic MonzaX USB",
    FALSE
    );

  gBS->RestoreTPL (OldTpl);

  DEBUG ((EFI_D_ERROR, "MonzaXDriverBindingStart: Exit\n"));

  return EFI_SUCCESS;

//
// Error handler
//
ErrorExit:
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    if (MonzaXDevice != NULL) {
      FreePool (MonzaXDevice);
      MonzaXDevice = NULL;
    }
  }

ErrorExit1:
  gBS->RestoreTPL (OldTpl);

  DEBUG ((EFI_D_ERROR, "MonzaXDriverBindingStart: Exit - %r\n", Status));
  return Status;
}


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
  )
{
  EFI_STATUS                  Status;
  MONZAX_DEV                  *MonzaXDevice;
  MONZAX_IO_PROTOCOL          *MonzaX;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gMonzaXIoProtocolGuid,
                  (VOID **) &MonzaX,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  MonzaXDevice = MONZAX_DEV_FROM_MONZAX_IO_PROTOCOL (MonzaX);

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gMonzaXIoProtocolGuid,
                  &MonzaXDevice->MonzaXIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  //
  // Free all resources.
  //
  if (MonzaXDevice->ControllerNameTable != NULL) {
    FreeUnicodeStringTable (MonzaXDevice->ControllerNameTable);
  }

  FreePool (MonzaXDevice);

  return EFI_SUCCESS;

}

/**
  Return MonzaX USB information.

  @param UsbIo  USB IO instance

  @return MonzaX USB information.
  @retval NULL This USB is not MonzaX USB device.
**/
MONZAX_USB_INFO *
IsMonzaXUsb (
  IN EFI_USB_IO_PROTOCOL  *UsbIo
  )
{
  EFI_STATUS                    Status;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;
  EFI_USB_DEVICE_DESCRIPTOR     DeviceDescriptor;
  UINTN                         Index;

  DEBUG ((EFI_D_INFO, "IsMonzaXUsb:\n"));

  //
  // Get the default interface descriptor
  //
  Status = UsbIo->UsbGetInterfaceDescriptor (
                    UsbIo,
                    &InterfaceDescriptor
                    );

  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Get the default interface descriptor
  //
  Status = UsbIo->UsbGetDeviceDescriptor (
                    UsbIo,
                    &DeviceDescriptor
                    );

  if (EFI_ERROR (Status)) {
    return NULL;
  }

  DEBUG ((EFI_D_INFO, "  IdVendor:  0x%04x\n", DeviceDescriptor.IdVendor));
  DEBUG ((EFI_D_INFO, "  IdProduct: 0x%04x\n", DeviceDescriptor.IdProduct));
  DEBUG ((EFI_D_INFO, "  DeviceClass:    0x%02x\n", DeviceDescriptor.DeviceClass));
  DEBUG ((EFI_D_INFO, "  DeviceSubClass: 0x%02x\n", DeviceDescriptor.DeviceSubClass));
  DEBUG ((EFI_D_INFO, "  DeviceProtocol: 0x%02x\n", DeviceDescriptor.DeviceProtocol));
  DEBUG ((EFI_D_INFO, "  InterfaceClass:    0x%02x\n", InterfaceDescriptor.InterfaceClass));
  DEBUG ((EFI_D_INFO, "  InterfaceSubClass: 0x%02x\n", InterfaceDescriptor.InterfaceSubClass));
  DEBUG ((EFI_D_INFO, "  InterfaceProtocol: 0x%02x\n", InterfaceDescriptor.InterfaceProtocol));

  for (Index = 0; Index < sizeof(mMonzaXUsbInfo)/sizeof(mMonzaXUsbInfo[0]); Index++) {
    if ((DeviceDescriptor.IdVendor == mMonzaXUsbInfo[Index].IdVendor) &&
        (DeviceDescriptor.IdProduct == mMonzaXUsbInfo[Index].IdProduct) &&
        (DeviceDescriptor.DeviceClass == mMonzaXUsbInfo[Index].DeviceClass) &&
        (DeviceDescriptor.DeviceSubClass == mMonzaXUsbInfo[Index].DeviceSubClass) &&
        (DeviceDescriptor.DeviceProtocol == mMonzaXUsbInfo[Index].DeviceProtocol) &&
        (InterfaceDescriptor.InterfaceClass == mMonzaXUsbInfo[Index].InterfaceClass) &&
        (InterfaceDescriptor.InterfaceSubClass == mMonzaXUsbInfo[Index].InterfaceSubClass) &&
        (InterfaceDescriptor.InterfaceProtocol == mMonzaXUsbInfo[Index].InterfaceProtocol)
        ) {
      return &mMonzaXUsbInfo[Index];
    }
  }

  return NULL;
}
