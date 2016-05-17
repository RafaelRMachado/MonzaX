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

#ifndef _MONZAX_IO_H_
#define _MONZAX_IO_H_

#define MONZAX_IO_PROTOCOL_GUID \
  { 0x56ec4783, 0x9d61, 0x49c5, 0x8c, 0x18, 0x17, 0x72, 0x35, 0x69, 0x93, 0xb6 }

typedef struct _MONZAX_IO_PROTOCOL MONZAX_IO_PROTOCOL;

#define MONZAX_I2C_DEVICE_ID_1          0x68
#define MONZAX_I2C_DEVICE_ID_2          0x6A
#define MONZAX_I2C_DEVICE_ID_3          0x6C
#define MONZAX_I2C_DEVICE_ID_4          0x6E
#define MONZAX_I2C_DEVICE_ID_DEFAULT    0x6E

#define MONZAX_SIZE_BYTES_RESERVED   22
#define MONZAX_SIZE_BYTES_EPC        18
#define MONZAX_SIZE_BYTES_TID        24
#define MONZAX_SIZE_BYTES_USER_2K    272
#define MONZAX_SIZE_BYTES_USER_8K    1024

#define MONZAX_BASE_ADDRESS_RESERVED   0x00
#define MONZAX_BASE_ADDRESS_EPC        0x16
#define MONZAX_BASE_ADDRESS_USER_2K    0x28
#define MONZAX_BASE_ADDRESS_USER_8K    0x40
#define MONZAX_BASE_ADDRESS_TID_2K     0x138
#define MONZAX_BASE_ADDRESS_TID_8K     0x28

typedef enum {
  MonzaXMemoryBankReserved,
  MonzaXMemoryBankEpc,
  MonzaXMemoryBankTid,
  MonzaXMemoryBankUser
} MONZAX_MEMORY_BANK_TYPE;

typedef enum {
  MonzaX2KDura = 0x140,
  MonzaX8KDura = 0x150
} MONZAX_CHIP_MODEL_TYPE;

typedef struct {
  UINT32                  Revision;
  UINT32                  Length;
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;
  UINT8                   I2cDeviceId;
} MONZAX_INFO;

#define MONZAX_INFO_REVISION 0x1

/**

  Get MonzaX chip information.

  @param This     Pointer to the MONZAX_IO_PROTOCOL instance.
  @param Info     The buffer to hold MonzaX chip information.

  @retval EFI_SUCCESS            The MonzaX information structure is returned.
  @retval EFI_INVALID_PARAMETER  Info is NULL.
  @retval EFI_BUFFER_TOO_SMALL   The Info buffer is too small to hold the full data.

**/
typedef
EFI_STATUS
(EFIAPI *MONZAX_IO_GET_INFO) (
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
typedef
EFI_STATUS
(EFIAPI *MONZAX_IO_SET_INFO) (
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
typedef
EFI_STATUS
(EFIAPI *MONZAX_IO_READ) (
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
typedef
EFI_STATUS
(EFIAPI *MONZAX_IO_WRITE) (
  IN  MONZAX_IO_PROTOCOL             *This,
  IN  UINT16                         Address,
  IN  UINT8                          *Data,
  IN  OUT UINTN                      *DataLen
  );

struct _MONZAX_IO_PROTOCOL {
  MONZAX_IO_GET_INFO                 GetInfo;
  MONZAX_IO_SET_INFO                 SetInfo;
  MONZAX_IO_READ                     Read;
  MONZAX_IO_WRITE                    Write;
};

extern EFI_GUID gMonzaXIoProtocolGuid;

#endif
