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

#include <Library\MonzaXLib.h>


UINT8 mMonzaxDeviceIds[] = { 0x68, 0x6A, 0x6C, 0x6E };

/**

  Get chip model type of MonzaX IO instance.

  @param MonzaXIo  MonzaX IO instance

  @return chip model type

**/
MONZAX_CHIP_MODEL_TYPE
GetChipModelType (
  IN MONZAX_IO_PROTOCOL      *MonzaXIo
  )
{
  EFI_STATUS   Status;
  MONZAX_INFO  Info;

  Info.Revision = MONZAX_INFO_REVISION;
  Info.Length   = sizeof(MONZAX_INFO);
  Status = MonzaXIo->GetInfo (MonzaXIo, &Info);
  if (EFI_ERROR(Status)) {
    return 0;
  }
  return Info.ChipModelType;
}

/**

  Convert a UINT8 array buffer to UINTN value.

  @param Buffer     UINT8 array.
  @param BufferLen  The length of UINT8 array.

  @return UINTN value
**/
UINTN
Array2Uint (
  IN UINT8 *Buffer,
  IN UINTN BufferLen
  )
{
  UINTN Index;
  UINTN Result;

  Result = 0;
  for (Index = 0; Index < BufferLen; Index++) {
    Result |= Buffer[Index];
    if (Index + Index < BufferLen) {
      Result = (Result << 8);
    }
  }

  return Result;
}

/**

  Convert a UINTN value to UINT8 array buffer.

  @param Value      UINTN value.
  @param Buffer     UINT8 array.
  @param BufferLen  The length of UINT8 array.

**/
VOID
Uint2Array (
  IN UINTN Value,
  IN UINT8 *Buffer,
  IN UINTN BufferLen
  )
{
  UINTN  Index;
  UINT8  *Ptr;

  Ptr = (UINT8 *)&Value;
  for (Index = 0; Index < BufferLen; Index++) {
    Buffer[Index] = Ptr[BufferLen - Index - 1];
  }
}

/**

  Set bit of data.

  @param Bit  The Bit number to be set
  @param Data The target data

**/
VOID
SetBit (
  IN UINT8 Bit,
  IN UINT8 *Data
  )
{
  (*Data) |= (1 << Bit);
}

/**

  Clear bit of data.

  @param Bit  The Bit number to be set
  @param Data The target data

**/
VOID
ClearBit (
  IN UINT8 Bit,
  IN UINT8 *Data
  )
{
  (*Data) &= ~(1 << Bit);
}

/**

  Read, modify, then write multiple bit values to a bank address.

  @param MonzaXIo  MonzaX IO instance
  @param Bank      The memory bank to write to
  @param Address   The memory bank address to write to
  @param BitCount  Count of bits
  @param BitNums   Number of bits
  @param BitValues Value of bits

  @return The number of bytes written

**/
UINTN
ReadModifyWrite (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo,
  IN MONZAX_MEMORY_BANK_TYPE       Bank,
  IN UINTN                         Address,
  IN UINT8                         BitCount,
  IN UINT8                         *BitNums,
  IN UINT8                         *BitValues
  )
{
  UINT8       Value;
  UINTN       Len;
  UINTN       Index;

  // Read the specified byte
  Len = MonzaxReadBank (MonzaXIo, Bank, Address, &Value, 1);

  // If the read failed
  // (this can happen if there is bus contention with RF)
  // Do not modify and rewrite
  if (Len > 0) {
    // Loop through the array of bits and bit values
    for (Index = 0; Index < BitCount; Index++) {
      if (BitValues[Index] == 0) {
        ClearBit (BitNums[Index], &Value);
      } else{
        SetBit (BitNums[Index], &Value);
      }
    }

    // Write the modified byte back to the chip
    return MonzaxWriteBank (MonzaXIo, Bank, Address, &Value, 1);
  } else {
    return 0;
  }
}

/**

  Read, modify, then write 1 bit values to a bank address.

  @param MonzaXIo  MonzaX IO instance
  @param Bank      The memory bank to write to
  @param Address   The memory bank address to write to
  @param BitNums   Number of bits
  @param BitValues Value of bits

  @return The number of bytes written

**/
UINTN
ReadModifyBitWrite (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo,
  IN MONZAX_MEMORY_BANK_TYPE       Bank,
  IN UINTN                         Address,
  IN UINT8                         BitNum,
  IN UINT8                         BitValue
  )
{
  return ReadModifyWrite (MonzaXIo, Bank, Address, 1, &BitNum, &BitValue);
}

/**

  Lock memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = lock, 1 = permalock
  @param UpperBit  The UpperBit of lock parameter

  @return The number of bytes written

**/
UINTN
LockBank (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm,
  IN UINT8                  UpperBit
  )
{
  UINT8 BitNums[2];
  UINT8 BitValues[2];

  BitNums[0] = UpperBit;
  BitNums[1] = UpperBit - 1;

  // Set the lock bit
  BitValues[0] = 1;

  // Conditionally set the permalock bit
  if (Perm != 0) {
    BitValues[1] = 1;
  } else {
    BitValues[1] = 0;
  }

  return ReadModifyWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x08, 2, BitNums, BitValues);
}

/**

  Unlock memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = unlock, 1 = perma-unlock
  @param UpperBit  The UpperBit of unlock parameter

  @return The number of bytes written

**/
UINTN
UnlockBank (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm,
  IN UINT8                  UpperBit
  )
{
  UINT8 BitNums[2];
  UINT8 BitValues[2];

  BitNums[0] = UpperBit;
  BitNums[1] = UpperBit - 1;

  // Clear the lock bit
  BitValues[0] = 0;

  // Conditionally set the perma-unlock bit
  if (Perm != 0) {
    BitValues[1] = 1;
  } else {
    BitValues[1] = 0;
  }

  return ReadModifyWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x08, 2, BitNums, BitValues);
}

/**

  Sets the EPC and adjusts the length bits.

  @param MonzaXIo  MonzaX IO instance
  @param Epc       The EPC
  @param EpcLen    The EPC length in bytes

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxSetEpc (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  *Epc,
  IN UINTN                  EpcLen
  )
{
  UINTN      Count;
  UINT8      Buffer;
  UINT8      Len;

  // EPC has to be a multiple of two bytes (one word)
  if ((EpcLen % 2) > 0) {
    return 0;
  }

  // Set the EPC
  Count = MonzaxWriteBank (MonzaXIo, MonzaXMemoryBankEpc, 2, Epc, EpcLen);

  // Adjust the length bits
  Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankEpc, 0, &Buffer, 1);
  if (Count > 0) {
    // Length is upper 5 bits
    Len = (UINT8)((Buffer & 0x07) | ((EpcLen / 2) << 3));
    Count = MonzaxWriteBank(MonzaXIo, MonzaXMemoryBankEpc, 0, &Len, 1);
  } else {
    // An error occurred. Return zero.
    Count = 0;
  }
  return Count;
}

/**

  Gets the EPC based on the length specified in the PC word.

  @param MonzaXIo   MonzaX IO instance
  @param Buffer     A buffer to hold the EPC
  @param BufferLen  The size of the buffer in bytes

  @return  The number of bytes read

**/
UINTN
EFIAPI
MonzaxGetEpc (
  IN MONZAX_IO_PROTOCOL *MonzaXIo,
  IN OUT UINT8          *Buffer,
  IN UINTN              BufferLen
  )
{
  UINTN   Count;
  UINT8   PcByte;
  UINT8   Len;

  // Read the first byte of the PC word
  Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankEpc, 0, &PcByte, 1);
  // Length is upper 5 bits
  Len = (PcByte >> 3) & 0x1F;
  // len contains the length of the EPC in 16-bit words
  if ((UINTN)(Len * 2) > BufferLen) {
    // Supplied buffer is not large enough to hold EPC
    return 0;
  }

  // Read the EPC
  Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankEpc, 2, Buffer, Len * 2);

  return Count;
}

/**

  Gets the TID of the chip.

  @param MonzaXIo   MonzaX IO instance
  @param Buffer     A buffer to hold the TID
  @param BufferLen  The size of the buffer in bytes

  @return  The number of bytes read

**/
UINTN
EFIAPI
MonzaxGetTid (
  IN MONZAX_IO_PROTOCOL *MonzaXIo,
  IN OUT UINT8          *Buffer,
  IN UINTN              BufferLen
  )
{
  UINTN                   Count;
  UINT8                   *Ptr;
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);

  Ptr = Buffer;
  // Supplied buffer is too small to hold TID
  if (BufferLen < 12) {
    return 0;
  }

  if (ChipModelType == MonzaX2KDura) {
    Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0x10, Ptr, 8);
    Ptr += 8;
    Count += MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0x00, Ptr, 4);
  } else {
    Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0x00, Ptr, 12);
  }

  return Count;
}

/**

  Locks the kill password.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = lock, 1 = permalock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxLockKillPw (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return LockBank (MonzaXIo, Perm, 7);
}

/**

  Unlocks the kill password.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = unlock, 1 = perma-unlock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxUnlockKillPw (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return UnlockBank (MonzaXIo, Perm, 7);
}

/**

  Locks the access password.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = lock, 1 = permalock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxLockAccessPw (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return LockBank (MonzaXIo, Perm, 5);
}

/**

  Unlocks the access password.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = unlock, 1 = perma-unlock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxUnlockAccessPw (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return UnlockBank (MonzaXIo, Perm, 5);
}

/**

  Locks the EPC memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = lock, 1 = permalock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxLockEpc (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return LockBank (MonzaXIo, Perm, 3);
}

/**

  Unlocks the EPC memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = unlock, 1 = perma-unlock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxUnlockEpc (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return UnlockBank (MonzaXIo, Perm, 3);
}

/**

  Locks the user memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = lock, 1 = permalock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxLockUser (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return LockBank (MonzaXIo, Perm, 1);
}

/**

  Unlocks the user memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Perm      0 = unlock, 1 = perma-unlock

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxUnlockUser (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Perm
  )
{
  return UnlockBank (MonzaXIo, Perm, 1);
}

/**

  Set the access password.

  @param MonzaXIo  MonzaX IO instance
  @param Password  The password to set.

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxSetAccessPw (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT32                 Password
  )
{
  UINT8 Buffer[4];

  Uint2Array (Password, Buffer, 4);

  // Write the access password to reserved memory.
  return MonzaxWriteBank (MonzaXIo, MonzaXMemoryBankReserved, 0x04, Buffer, 4);
}

/**

  Set the kill password.

  @param MonzaXIo  MonzaX IO instance
  @param Password  The password to set.

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxSetKillPw (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT32                 Password
  )
{
  UINT8 Buffer[4];

  Uint2Array (Password, Buffer, 4);

  // Write the kill password to reserved memory.
  return MonzaxWriteBank (MonzaXIo, MonzaXMemoryBankReserved, 0x00, Buffer, 4);
}

/**

  Performs a basic, non-destructive test to determine if the chip is present.
  We do this by reading the Class ID from TID memory.
  The value read should be 0xE2 (Gen2).

  @param MonzaXIo  MonzaX IO instance

  @retval  0  chip present
  @retval  1  chip did not respond

**/
UINT8
EFIAPI
MonzaxChipTest (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  UINT8                   Buffer;
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);
  if (ChipModelType == MonzaX2KDura) {
    MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0x10, &Buffer, 1);
  } else {
    MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0x00, &Buffer, 1);
  }

  // Read the chip's Class ID from the TID bank.
  // It should be 0xE2 (Gen2)
  if (Buffer == 0xE2) {
    return 0;
  } else {
    return 1;
  }
}

/**

  Permalocks the specified block of user memory.
  This is permenant for all blocks except block zero.
  The memory cannot be unlocked via I2C or RF.

  @param MonzaXIo  MonzaX IO instance
  @param Block     The block of user memory to lock.

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxBlockPermalock (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Block
  )
{
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);
  if (ChipModelType == MonzaX2KDura) {
    if (Block > 4) {
      return EFI_UNSUPPORTED;
    }
    return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x09, (7 - Block), 1 );
  } else {
    if (Block < 8) {
      return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x12, (7 - Block), 1);
    } else if ((Block >= 8) && (Block < 16)) {
      return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x13, (7 - (Block-8)), 1);
    } else {
      return 0;
    }
  }
}

/**

  Unlocks the specified block of user memory.
  This is only possible for block zero.

  @param MonzaXIo  MonzaX IO instance
  @param Block     The block of user memory to lock.
                   Zero is the only valid value for now.

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxBlockUnlock (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Block
  )
{
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);

  // Block zero is the only block you can unlock.
  if (Block != 0) {
    return EFI_UNSUPPORTED;
  }

  if (ChipModelType == MonzaX2KDura) {
    return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x09, 7, 0);
  } else {
    return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x12, 7, 0);
  }
}

/**

  Permanently locks the I2C device address.
  This only supported by Monza X 8K Dura.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxLockI2cDeviceId (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);
  if (ChipModelType == MonzaX8KDura) {
    return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x09, 7, 1);
  } else {
    return 0;
  }
}

/**

  Reads the chip model number.

  @param MonzaXIo  MonzaX IO instance

  @return  The model number

**/
UINT16
EFIAPI
MonzaxReadModelNumber (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo
  )
{
  // Read the chip model number from the TID bank
  // Model number is 12 bits
  UINT8                   Buffer[2];
  UINT16                  Model;
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);

  Buffer[0] = 0;
  Buffer[1] = 0;
  if (ChipModelType == MonzaX2KDura) {
    MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0x12, Buffer, 2);
  } else {
    MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0x02, Buffer, 2);
  }

  // Drop the upper 4 bits
  Model = (UINT16)(0x0FFF & Array2Uint (Buffer, 2));
  return Model;
}

/**

  Enables RF access to the chip when DC power is applied.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableRfDci (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 2, 1);
}

/**

  Disables RF access to the chip when DC power is applied.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableRfDci (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 2, 0);
}

/**

  Enables RF port #1.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableRfPort1 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 0, 0);
}

/**

  Disables RF port #1.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableRfPort1 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 0, 1);
}

/**

  Enables RF port #2.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableRfPort2 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 1, 0);
}

/**

  Disables RF port #2.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableRfPort2 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 1, 1);
}

/**

  Enables write wakeup mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableWriteWakeupMode (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 6, 1);
}

/**

  Disables write wakeup mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableWriteWakeupMode (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 6, 0);
}

/**

  Enables block permalocking over RF.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableBlockPermlock (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 5, 1);
}

/**

  Disables block permalocking over RF.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableBlockPermlock (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 5, 0);
}

/**

  Enables QT. Puts the tag into public mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableQt (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 3, 1);
}

/**

  Disable QT. Puts the tag into private mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableQt (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 3, 0);
}

/**

  Enables QT short range.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableQtShortRange (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 4, 1);
}

/**

  Disable QT short range.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableQtShortRange (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x15, 4, 0);
}

/**

  Kills the tag. This can be undone over I2C only (not RF).

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxKillTag (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x09, 2, 1);
}

/**

  Un-kills the tag. This can only be done over I2C (not RF).

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxUnkillTag (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  return ReadModifyBitWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x09, 2, 0);
}

/**

  Sets the I2C device ID.

  @param MonzaXIo     MonzaX IO instance
  @param I2cDeviceId  The device ID (must be one of the four valid IDs)

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxSetI2cDeviceId (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  I2cDeviceId
  )
{
  UINTN                   Count;
  UINT8                   BitNums[2];
  UINT8                   BitValues[2];
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);

  BitNums[0] = 1;
  BitNums[1] = 0;

  // Set the ID bits that correspond to
  // the supplied I2C device ID
  switch(I2cDeviceId) {
  case 0x68:
    BitValues[0] = 0;
    BitValues[1] = 0;
    break;
  case 0x6A:
    BitValues[0] = 0;
    BitValues[1] = 1;
    break;
  case 0x6C:
    BitValues[0] = 1;
    BitValues[1] = 0;
    break;
  case 0x6E:
    BitValues[0] = 1;
    BitValues[1] = 1;
    break;
  default:
    // Unsupported ID
    return 1;
  }
  Count = ReadModifyWrite (MonzaXIo, MonzaXMemoryBankReserved, 0x09, 2, BitNums, BitValues);

  // Make this the active I2C device
  MonzaxSetActiveDevice (MonzaXIo, ChipModelType, I2cDeviceId);

  return Count;
}

/**

  Initializes the Monza X API.

  @param MonzaXIo MonzaX IO instance

  @retval 0         Success
  @retval Non-Zero  Error

**/
UINT8
EFIAPI
MonzaxInitialize (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo
  )
{
  return 0;
}

/**

  Shuts down the Monza X API and performs any necessary cleanup.

  @param MonzaXIo MonzaX IO instance

  @retval 0         Success
  @retval Non-Zero  Error

**/
UINT8
EFIAPI
MonzaxShutdown (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo
  )
{
  return 0;
}

/**

  Returns the size of the specified bank in bytes.

  @param MonzaXIo  MonzaX IO instance
  @param Bank      The memory bank

  @return  The size of the memory bank in bytes

**/
UINTN
EFIAPI
MonzaxGetBankSize (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo,
  IN MONZAX_MEMORY_BANK_TYPE       Bank
  )
{
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);
  switch (Bank) {
  case MonzaXMemoryBankReserved:
    return MONZAX_SIZE_BYTES_RESERVED;
  case MonzaXMemoryBankEpc:
    return MONZAX_SIZE_BYTES_EPC;
  case MonzaXMemoryBankTid:
    return MONZAX_SIZE_BYTES_TID;
  case MonzaXMemoryBankUser:
    if (ChipModelType == MonzaX2KDura) {
      return MONZAX_SIZE_BYTES_USER_2K;
    } else {
      return MONZAX_SIZE_BYTES_USER_8K;
    }
  }
  return 0;
}

/**

  Returns the base address of the specified bank.

  @param MonzaXIo  MonzaX IO instance
  @param Bank      The memory bank

  @return  The base address of the memory bank

**/
UINT16
EFIAPI
MonzaxGetBankBaseAddress (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo,
  IN MONZAX_MEMORY_BANK_TYPE       Bank
  )
{
  MONZAX_CHIP_MODEL_TYPE  ChipModelType;

  ChipModelType = GetChipModelType(MonzaXIo);
  switch (Bank) {
  case MonzaXMemoryBankReserved:
    return MONZAX_BASE_ADDRESS_RESERVED;
  case MonzaXMemoryBankEpc:
    return MONZAX_BASE_ADDRESS_EPC;
  case MonzaXMemoryBankTid:
    if (ChipModelType == MonzaX2KDura) {
      return MONZAX_BASE_ADDRESS_TID_2K;
    } else {
      return MONZAX_BASE_ADDRESS_TID_8K;
    }
  case MonzaXMemoryBankUser:
    if (ChipModelType == MonzaX2KDura) {
      return MONZAX_BASE_ADDRESS_USER_2K;
    } else {
      return MONZAX_BASE_ADDRESS_USER_8K;
    }
  }
  return 0;
}

/**

  Sets the specified I2C device as active.

  @param MonzaXIo       MonzaX IO instance
  @param Model          The Monza X model (2k, 8K)
  @param I2cDeviceId    The I2C device ID to make active

  @retval 0         Success
  @retval Non-Zero  Error

**/
UINT8
EFIAPI
MonzaxSetActiveDevice (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN MONZAX_CHIP_MODEL_TYPE Model,
  IN UINT8                  I2cDeviceId
  )
{
  MONZAX_INFO  MonzaxInfo;

  MonzaxInfo.Revision      = MONZAX_INFO_REVISION;
  MonzaxInfo.Length        = sizeof(MonzaxInfo);
  MonzaxInfo.I2cDeviceId   = I2cDeviceId;
  MonzaxInfo.ChipModelType = Model;
  MonzaXIo->SetInfo (MonzaXIo, &MonzaxInfo);

  return 0;
}

/**

  Looks for Monza X chips on the I2C bus.
  The first one found is set to the active device.

  @param MonzaXIo       MonzaX IO instance
  @param Model          Will be populated with the Monza X model detected (2k, 8K)
  @param I2cDeviceId    Will be populated with the I2C device ID detected

  @retval 0         Success
  @retval Non-Zero  Error

**/
UINT8
EFIAPI
MonzaxAutoDetectChip (
  IN MONZAX_IO_PROTOCOL      *MonzaXIo,
  OUT MONZAX_CHIP_MODEL_TYPE *Model,
  OUT UINT8                  *I2cDeviceId
  )
{
  UINTN  NumIds;
  UINTN  Index;

  NumIds = sizeof(mMonzaxDeviceIds);

  for (Index = 0; Index < NumIds; Index++) {
    MonzaxSetActiveDevice (MonzaXIo, MonzaX2KDura, mMonzaxDeviceIds[Index]);
    if (MonzaxChipTest (MonzaXIo) == 0) {
      *Model = MonzaX2KDura;
      *I2cDeviceId = mMonzaxDeviceIds[Index];
      return 0;
    }
    MonzaxSetActiveDevice (MonzaXIo, MonzaX8KDura, mMonzaxDeviceIds[Index]);
    if (MonzaxChipTest (MonzaXIo) == 0) {
      *Model = MonzaX8KDura;
      *I2cDeviceId = mMonzaxDeviceIds[Index];
      return 0;
    }
  }

  return (UINT8)-1;
}

/**

  Write to the specified memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Bank      The memory bank to write to
  @param Offset    The offset in bytes to begin writing
  @param Data      A buffer of data to write
  @param DataLen   The number of bytes to write.

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxWriteBank (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo,
  IN MONZAX_MEMORY_BANK_TYPE       Bank,
  IN UINTN                         Offset,
  IN UINT8                         *Data,
  IN UINTN                         DataLen
  )
{
  UINT16      Address;
  EFI_STATUS  Status;

  Address = (UINT16)(MonzaxGetBankBaseAddress (MonzaXIo, Bank) + Offset);

  Status = MonzaXIo->Write (MonzaXIo, Address, Data, &DataLen);
  if (EFI_ERROR(Status)) {
    return 0;
  }
  return DataLen;
}

/**

  Reads from the specified memory bank.

  @param MonzaXIo  MonzaX IO instance
  @param Bank      The memory bank to read from
  @param Offset    The offset in bytes to begin reading
  @param Data      A buffer to hold the data read
  @param DataLen   The number of bytes to read

  @return  The number of bytes read

**/
UINTN
EFIAPI
MonzaxReadBank (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo,
  IN MONZAX_MEMORY_BANK_TYPE       Bank,
  IN UINTN                         Offset,
  OUT UINT8                        *Data,
  IN UINTN                         DataLen
  )
{
  UINT16      Address;
  EFI_STATUS  Status;

  Address = (UINT16)(MonzaxGetBankBaseAddress (MonzaXIo, Bank) + Offset);

  Status = MonzaXIo->Read (MonzaXIo, Address, Data, &DataLen);
  if (EFI_ERROR(Status)) {
    return 0;
  }
  return DataLen;
}

