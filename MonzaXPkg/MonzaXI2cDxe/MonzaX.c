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

typedef struct {
  ///
  /// Number of elements in the operation array
  ///
  UINTN OperationCount;

  ///
  /// Description of the I2C operation
  ///
  EFI_I2C_OPERATION Operation[2];
} I2C_REQUEST_PACKET_WRITE_READ;


#define DUMP_ASCII

/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpData (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;
  for (Index = 0; Index < Size; Index++) {
    DEBUG ((EFI_D_INFO, "%02x ", (UINTN)Data[Index]));
  }
}

/**

  Return if char is printable.

  @param Ch A Character

  @retval TRUE  If the char is printable.
  @retval FALSE If the char is not printable.

**/
STATIC
BOOLEAN
IsPrintable (
  IN UINT8 Ch
  )
{
  if ((Ch <= 0x20) || (Ch >= 0x7F)) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN   Index;
  UINTN   Count;
  UINTN   Left;
#ifdef DUMP_ASCII
  UINTN   Index2;
#endif

#define COLUME_SIZE  (16)

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
//    DEBUG ((EFI_D_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
#ifdef DUMP_ASCII
    DEBUG ((EFI_D_INFO, "        '"));
    for ( Index2 = 0; Index2 < COLUME_SIZE; Index2++ ) {
      DEBUG ((EFI_D_INFO, "%c", IsPrintable((Data + Index * COLUME_SIZE)[Index2]) ? (Data + Index * COLUME_SIZE)[Index2] : '.'));
    }
    DEBUG ((EFI_D_INFO, "'"));
#endif
    DEBUG ((EFI_D_INFO, "\n"));
  }

  if (Left != 0) {
//    DEBUG ((EFI_D_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
#ifdef DUMP_ASCII
    for ( Index2 = 0; Index2 < COLUME_SIZE - Left; Index2++ ) {
      DEBUG ((EFI_D_INFO, "   "));
    }
    DEBUG ((EFI_D_INFO, "        '"));
    for ( Index2 = 0; Index2 < Left; Index2++ ) {
      DEBUG ((EFI_D_INFO, "%c", IsPrintable((Data + Index * COLUME_SIZE)[Index2]) ? (Data + Index * COLUME_SIZE)[Index2] : '.'));
    }
    DEBUG ((EFI_D_INFO, "'"));
#endif
    DEBUG ((EFI_D_INFO, "\n"));
  }
}

/**
  Get I2C slave address index according to MonzaxI2cDeviceId.

  @param Dev        Pointer to the MONZAX_DEV instance.

  @return I2C slave address index according to MonzaxI2cDeviceId.
**/
UINTN
GetSlaveAddressIndex (
  IN MONZAX_DEV           *Dev
  )
{
  UINTN  Index;

  for (Index = 0; Index < Dev->I2cDevice->SlaveAddressCount; Index ++) {
    if ((UINT8)Dev->I2cDevice->SlaveAddressArray[Index] == Dev->MonzaxI2cDeviceId) {
      return Index;
    }
  }
  
  // Invalid Index
  return Dev->I2cDevice->SlaveAddressCount;
}

/**

  Read from an I2C device.
  
  @param Dev        Pointer to the MONZAX_DEV instance.
  @param Address    The memory address to start reading from.
  @param AddressLen The length of the address in bytes (1 or 2).
  @param Data       A buffer of bytes to hold the data.
  @param DataLen    The number of bytes to read.

  @return  The number of bytes read.

**/
UINTN
I2cRead (
  IN MONZAX_DEV           *Dev,
  IN UINT16               Address,
  IN UINT8                AddressLen,
  OUT UINT8               *Data,
  IN UINTN                DataLen
  )
{
  EFI_STATUS                       Status;
  I2C_REQUEST_PACKET_WRITE_READ    Request;
  UINT8                            Addr[2];

  DEBUG ((EFI_D_INFO, "I2cRead - Addr (0x%x), Size (0x%x)\n", Address, DataLen));

  if (AddressLen == 1) {
    // 8-bit memory address
    Addr[0] = (UINT8) Address;
  } else {
    // 16-bit memory addresss
    Addr[0] = (UINT8) (Address >> 8) & 0xFF;
    Addr[1] = (UINT8) Address & 0xFF;
  }

  Request.OperationCount = 2;
  Request.Operation[0].Flags = 0; // ~I2C_FLAG_READ
  Request.Operation[0].LengthInBytes = (UINT32)AddressLen;
  Request.Operation[0].Buffer = Addr;
  Request.Operation[1].Flags = I2C_FLAG_READ;
  Request.Operation[1].LengthInBytes = (UINT32)DataLen;
  Request.Operation[1].Buffer = Data;

  Status = Dev->I2cIo->QueueRequest (
                             Dev->I2cIo,
                             GetSlaveAddressIndex (Dev),
                             NULL,
                             (EFI_I2C_REQUEST_PACKET *)&Request,
                             NULL
                             );

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "I2cRead - %r\n", Status));
    return 0;
  }

  return DataLen;
}

/**

  Write to an I2C device. 

  @param Dev        Pointer to the MONZAX_DEV instance.
  @param Address    The memory address to start writing to.
  @param AddressLen The length of the address in bytes (1 or 2).
  @param Data       The words to write.
  @param DataLen    The number of words to write.

  @return The number of words written.

**/
UINTN
I2cWrite (
  IN MONZAX_DEV           *Dev,
  IN UINT16               Address,
  IN UINT8                AddressLen,
  IN UINT16               *Data,
  IN UINTN                DataLen
  )
{
  EFI_STATUS                Status;
  EFI_I2C_REQUEST_PACKET    Request;
  UINTN                     Index;
  UINTN                     NewDataLen;
  UINT8                     *OldBuf;
  UINT8                     *NewBuf;

  DEBUG ((EFI_D_INFO, "I2cWrite - Addr (0x%x), Size (0x%x)\n", Address, DataLen));

  OldBuf = (UINT8 *)Data;
  NewDataLen = (DataLen * 2) + AddressLen;
  NewBuf = (UINT8 *)AllocatePool(NewDataLen);

  if (AddressLen == 1) {
    // 8-bit memory address
    NewBuf[0] = (UINT8) Address;
  } else {
    // 16-bit memory addresss
    NewBuf[0] = (UINT8) (Address >> 8) & 0xFF;
    NewBuf[1] = (UINT8) Address & 0xFF;
  }

  // Copy the data into the new buffer
  for (Index = 0; Index < NewDataLen - AddressLen; Index++) {
    NewBuf[Index + AddressLen] = OldBuf[Index];
  }

  Request.OperationCount = 1;
  Request.Operation[0].Flags = 0; // ~I2C_FLAG_READ
  Request.Operation[0].LengthInBytes = (UINT32)NewDataLen;
  Request.Operation[0].Buffer = NewBuf;

  Status = Dev->I2cIo->QueueRequest (
                             Dev->I2cIo,
                             GetSlaveAddressIndex (Dev),
                             NULL,
                             &Request,
                             NULL
                             );
  FreePool (NewBuf);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "I2cWrite - %r\n", Status));
    return 0;
  }

  return DataLen;
}

/**

  Write data to adjusted address.

  @param Dev        Pointer to the MONZAX_DEV instance.
  @param Address    The memory address to start writing to.
  @param Data       The words to write.
  @param DataLen    The number of words to write.

  @return The number of words written.

**/
UINTN
WriteAdjustedAddress (
  IN MONZAX_DEV           *Dev,
  IN UINT16               Address,
  IN UINT16               *Data,
  IN UINTN                DataLen
  )
{
  UINTN      Count;
  UINTN      Len;
  UINT8      AddressLen;

  // Handle dual address requirement of Monza X 2K Dura
  if (Dev->ChipModelType == MonzaX2KDura) {
    AddressLen = 1;

    // Is the supplied address larger than a byte?
    // If so, change the device id and adjust the address.
    // The lower bit of the device id is the upper bit of the address.
    if (Address > 0xFF) {
      Dev->MonzaxI2cDeviceId ++;
      Address -= 0x0100;
      Count = I2cWrite (Dev, Address, AddressLen, Data, DataLen);
      Dev->MonzaxI2cDeviceId --;
    }
    // Will the write operation cross the address boundary (0xFF)?
    // If  so, write the data in two chunks. This prevents addressing
    // problems if the end user has implemented i2c_write using a
    // single word write.
    else if (Address + (DataLen - 1) > 0xFF) {
      Len = 0xFF - Address + 1;
      Count = I2cWrite (Dev, Address, AddressLen, Data, Len);
      Dev->MonzaxI2cDeviceId ++;
      Count += I2cWrite (Dev, (UINT16)(Address + Len), AddressLen, Data + Len, DataLen - Len);
      Dev->MonzaxI2cDeviceId --;
    }
    // Regular write operation.
    else {
      Count = I2cWrite(Dev, Address, AddressLen, Data, DataLen);
    }
  }
  // Monza X 8K Dura. Regular write operation.
  else {
    AddressLen = 2;
    Count = I2cWrite (Dev, Address, AddressLen, Data, DataLen);
  }
  return Count;
}

/**

  Read data from adjusted address.

  @param Dev        Pointer to the MONZAX_DEV instance.
  @param Address    The memory address to start reading from.
  @param Data       A buffer of bytes to hold the data.
  @param DataLen    The number of bytes to read.

  @return  The number of bytes read.

**/
UINTN
ReadAdjustedAddress (
  IN MONZAX_DEV           *Dev,
  IN UINT16               Address,
  OUT UINT8               *Data,
  IN UINTN                DataLen
  )
{
  UINTN      Count;
  UINT8      AddressLen;
  UINTN      Len;

  // Handle dual address requirement of Monza X 2K Dura
  if (Dev->ChipModelType == MonzaX2KDura) {
    AddressLen = 1;

    // Is the supplied address larger than a byte?
    // If so, change the device id and adjust the address.
    // The lower bit of the device id is the upper bit of the address.
    if (Address > 0xFF) {
      Dev->MonzaxI2cDeviceId ++;
      Address -= 0x0100;
      Count = I2cRead (Dev, Address, AddressLen, Data, DataLen);
      Dev->MonzaxI2cDeviceId --;
    }
    // Will the read operation cross the address boundary (0xFF)?
    // If  so, read the data in two chunks. This prevents addressing
    // problems if the end user has implemented i2c_read using a
    // single byte read.
    else if (Address + (DataLen - 1) > 0xFF) {
      Len = 0xFF - Address + 1;
      Count = I2cRead (Dev, Address, AddressLen, Data, Len);

      Dev->MonzaxI2cDeviceId ++;
      Count += I2cRead (Dev, (UINT16)(Address + Len), AddressLen, Data + Len, DataLen - Len);
      Dev->MonzaxI2cDeviceId --;
    }
    // Regular read operation.
    else {
      Count = I2cRead (Dev, Address, AddressLen, Data, DataLen);
    }
  }
  // Monza X 8K Dura. Regular read operation.
  else {
    AddressLen = 2;
    Count = I2cRead (Dev, Address, AddressLen, Data, DataLen);
  }

  return Count;
}

/**

  Write data to MonzaX chip.

  @param Dev        Pointer to the MONZAX_DEV instance.
  @param Address    The device address of MonzaX chip on where the data is written to.
  @param Data       A pointer to the buffer of data that will be written to MonzaX device.
  @param DataLen    The size, in bytes, of the data buffer specified by Data.

  @return The amount of data actually transferred.

**/
UINTN
MonzaxWriteAddress (
  IN MONZAX_DEV           *Dev,
  IN UINT16               Address,
  IN UINT8                *Data,
  IN UINTN                DataLen
  )
{
  UINTN      Count;
  UINT8      *Ptr;

  Count = 0;

  // Copy the data pointer.
  Ptr = Data;

  // Check if write starts on a word boundary.
  if ((Address % 2) > 0) {
    UINT8 FirstWord[2];
    // Write does not start on a word boundary.
    // Read the previous byte.
    ReadAdjustedAddress (Dev, Address - 1, FirstWord, 1);
    // Append the previous byte with the first byte of data.
    // This is the first word. Write it to memory.
    FirstWord[1] = *Ptr;
    Count += WriteAdjustedAddress (Dev, Address - 1, (UINT16 *) FirstWord, 1);
    // Adjust the address, data pointer and length.
    Address++;
    Ptr++;
    DataLen--;
  }

  // Write one word at a time.
  while (DataLen > 1) {
    Count += WriteAdjustedAddress (Dev, Address, (UINT16*) Ptr, 1);
    // Adjust the address, data pointer and length.
    Address += 2;
    DataLen -= 2;
    Ptr += 2;
  }

  // Handle partial word.
  if (DataLen > 0) {
    UINT8 LastWord[2];
    // Write does not end on a word boundary.
    // Read the next byte.
    LastWord[0] = *Ptr;
    ReadAdjustedAddress (Dev, Address + 1, LastWord + 1, 1);
    // This is the last word. Write it.
    Count += WriteAdjustedAddress (Dev, Address, (UINT16 *) LastWord, 1);
  }

  // Count is in words. Return bytes.
  return Count * 2;
}

/**

  Read data from MonzaX chip.

  @param Dev        Pointer to the MONZAX_DEV instance.
  @param Address    The device address of MonzaX chip on where the data is read from.
  @param Data       A pointer to the buffer of data that will be read from MonzaX device.
  @param DataLen    The size, in bytes, of the data buffer specified by Data.

  @return  The amount of data actually transferred.

**/
UINTN
MonzaxReadAddress (
  IN MONZAX_DEV           *Dev,
  IN UINT16               Address,
  OUT UINT8               *Data,
  IN UINTN                DataLen
  )
{
  return ReadAdjustedAddress (Dev, Address, Data, DataLen);
}

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
  )
{
  MONZAX_DEV           *Dev;

  Dev = MONZAX_DEV_FROM_MONZAX_IO_PROTOCOL (This);

  if (Info == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Info->Length < sizeof(MONZAX_INFO)) {
    Info->Length = sizeof(MONZAX_INFO);
    return EFI_BUFFER_TOO_SMALL;
  }
  Info->Revision = MONZAX_INFO_REVISION;
  Info->Length = sizeof(MONZAX_INFO);
  Info->I2cDeviceId = Dev->MonzaxI2cDeviceId;
  Info->ChipModelType = Dev->ChipModelType;

  return EFI_SUCCESS;
}

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
  )
{
  MONZAX_DEV           *Dev;

  Dev = MONZAX_DEV_FROM_MONZAX_IO_PROTOCOL(This);

  if (Info == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Info->Length < sizeof(MONZAX_INFO)) {
    return EFI_BUFFER_TOO_SMALL;
  }
  Dev->MonzaxI2cDeviceId = Info->I2cDeviceId;
  Dev->ChipModelType = Info->ChipModelType;

  return EFI_SUCCESS;
}

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
  )
{
  MONZAX_DEV           *Dev;
  UINTN                TransferDataLen;
  UINTN                ExpectDataLen;

  Dev = MONZAX_DEV_FROM_MONZAX_IO_PROTOCOL (This);
  ExpectDataLen = *DataLen;
  TransferDataLen = MonzaxReadAddress (Dev, Address, Data, *DataLen);
  *DataLen = TransferDataLen;
  if (TransferDataLen == 0) {
    return EFI_DEVICE_ERROR;
  } else {
    ASSERT (TransferDataLen <= ExpectDataLen);
    return EFI_SUCCESS;
  }
}

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
  )
{
  MONZAX_DEV           *Dev;
  UINTN                TransferDataLen;
  UINTN                ExpectDataLen;

  Dev = MONZAX_DEV_FROM_MONZAX_IO_PROTOCOL (This);
  ExpectDataLen = *DataLen;
  TransferDataLen = MonzaxWriteAddress (Dev, Address, Data, *DataLen);
  *DataLen = TransferDataLen;
  if (TransferDataLen == 0) {
    return EFI_DEVICE_ERROR;
  } else {
    ASSERT (TransferDataLen <= ExpectDataLen);
    return EFI_SUCCESS;
  }
}
