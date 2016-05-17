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


/**
  Entrypoint of I2C MonzaX Driver.

  This function is the entrypoint of I2C MonzaX Driver. It installs Driver Binding
  Protocols together with Component Name Protocols.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
MonzaXDriverBindingEntryPoint (
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
  Check whether I2C MonzaX driver supports this device.

  @param  This                   The I2C MonzaX driver binding protocol.
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
  EFI_I2C_IO_PROTOCOL *I2cIo;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cIoProtocolGuid,
                  (VOID **) &I2cIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Use the DevicePath Protocol interface to check whether Controller is
  // a MonzaX device that can be managed by this driver.
  //
  Status = EFI_SUCCESS;
  if (!IsMonzaXI2c (I2cIo)) {
    Status = EFI_UNSUPPORTED;
  } else {
    DEBUG ((EFI_D_ERROR, "MonzaXDriverBindingSupported: Find!\n"));
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiI2cIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}


/**
  Starts the MonzaX device with this driver.

  This function consumes I2C Bus Portocol, intializes I2C MonzaX device,
  installs MonzaX IO Protocol.

  @param  This                  The I2C MonzaX driver binding instance.
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
  EFI_I2C_IO_PROTOCOL         *I2cIo;
  MONZAX_DEV                  *MonzaXDevice;
  EFI_DEVICE_PATH             *DevicePath;
  EFI_TPL                     OldTpl;
  UINT8                       Result;

  DEBUG ((EFI_D_ERROR, "MonzaXDriverBindingStart: Enter\n"));

  MonzaXDevice = NULL;

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);
  //
  // Open I2C Bus Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cIoProtocolGuid,
                  (VOID **) &I2cIo,
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
  MonzaXDevice->I2cIo             = I2cIo;
  MonzaXDevice->I2cDevice         = GetMonzaXI2cDevice (I2cIo);
  ASSERT (MonzaXDevice->I2cDevice != NULL);
  CopyMem (&MonzaXDevice->MonzaXIo, &mMonzaXIo, sizeof(mMonzaXIo));
  MonzaXDevice->DevicePath        = DevicePath;
  MonzaXDevice->ControllerHandle  = Controller;

  DEBUG ((EFI_D_INFO, "MonzaxChipTest\n"));
  // Hardcode
  MonzaXDevice->MonzaxI2cDeviceId = MONZAX_I2C_DEVICE_ID_DEFAULT;
  MonzaXDevice->ChipModelType = MonzaX2KDura;
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
    L"Generic MonzaX I2C",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gMonzaXComponentName2.SupportedLanguages,
    &MonzaXDevice->ControllerNameTable,
    L"Generic MonzaX I2C",
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
          &gEfiI2cIoProtocolGuid,
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
  Stop the I2C MonzaX device handled by this driver.

  @param  This                   The I2C MonzaX driver binding protocol.
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
         &gEfiI2cIoProtocolGuid,
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
  Return if this is MonzaX I2C device.

  @param I2cIo  I2C IO instance

  @retval TRUE  This is MonzaX I2C device.
  @retval FALSE This is not MonzaX I2C device.
**/
BOOLEAN
IsMonzaXI2c (
  IN EFI_I2C_IO_PROTOCOL *I2cIo
  )
{
  DEBUG ((EFI_D_INFO, "I2C DeviceGuid - %g\n", I2cIo->DeviceGuid));
  if (CompareGuid (&gMonzaXI2cDeviceGuid, I2cIo->DeviceGuid)) {
    DEBUG ((EFI_D_INFO, "MonzaX found!!!\n"));
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Return MonzaX I2C device structure.

  @param I2cIo             I2C IO instance

  @return MonzaX I2C device structure.
  @retval NULL This is not MonzaX I2C device.
**/
EFI_I2C_DEVICE *
GetMonzaXI2cDevice (
  IN EFI_I2C_IO_PROTOCOL *I2cIo
  )
{
  EFI_STATUS                 Status;
  EFI_I2C_ENUMERATE_PROTOCOL *I2cEnumerate;
  CONST EFI_I2C_DEVICE       *Device;
  UINTN                      NoHandles;
  EFI_HANDLE                 *HandleBuffer;
  UINTN                      Index;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiI2cEnumerateProtocolGuid,
                  NULL,
                  &NoHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiI2cEnumerateProtocolGuid, (VOID **)&I2cEnumerate);
    if (EFI_ERROR(Status)) {
      continue;
    }

    //
    //  Walk the list of I2C devices on this bus
    //
    Device = NULL;
    while (TRUE) {
      //
      //  Get the next I2C device
      //
      Status = I2cEnumerate->Enumerate (I2cEnumerate, &Device);
      if (EFI_ERROR(Status) || (Device == NULL)) {
        Device = NULL;
        break;
      }
      //
      //  Determine if the device info is valid
      //
      if ((Device->DeviceGuid == NULL) || (Device->SlaveAddressCount == 0) || (Device->SlaveAddressArray == NULL)) {
        continue;
      }

      //
      // Check if it matches current one.
      //
      if (CompareGuid(Device->DeviceGuid, I2cIo->DeviceGuid) && (Device->DeviceIndex == I2cIo->DeviceIndex)) {
        //
        // Found
        //
        break;
      }
    }
    //
    // If found, stop and return
    //
    if (Device != NULL) {
      return (EFI_I2C_DEVICE *)Device;
    }
  }
  return NULL;
}
