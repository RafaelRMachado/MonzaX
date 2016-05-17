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

#ifndef _MONZAX_LIB_H_
#define _MONZAX_LIB_H_

#include <Uefi.h>
#include <Protocol/MonzaXIo.h>

/**

  Initializes the Monza X API.

  @param MonzaXIo MonzaX IO instance

  @retval 0         Success
  @retval Non-Zero  Error

**/
UINT8
EFIAPI
MonzaxInitialize (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo
  );

/**

  Shuts down the Monza X API and performs any necessary cleanup.

  @param MonzaXIo MonzaX IO instance

  @retval 0         Success
  @retval Non-Zero  Error

**/
UINT8
EFIAPI
MonzaxShutdown (
  IN MONZAX_IO_PROTOCOL            *MonzaXIo
  );

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
  );

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
  );

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
  );

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
  );

/**

  Disables RF access to the chip when DC power is applied.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableRfDci (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Enables RF access to the chip when DC power is applied.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableRfDci (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Disables RF port #1.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableRfPort1 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Enables RF port #1.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableRfPort1 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Disables RF port #2.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableRfPort2 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Enables RF port #2.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableRfPort2 (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

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
  );

/**

  Reads the chip model number.

  @param MonzaXIo  MonzaX IO instance

  @return  The model number

**/
UINT16
EFIAPI
MonzaxReadModelNumber (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo
  );

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
  IN MONZAX_IO_PROTOCOL     *MonzaXIo
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**

  Kills the tag. This can be undone over I2C only (not RF).

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxKillTag (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Un-kills the tag. This can only be done over I2C (not RF).

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxUnkillTag (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Enables QT. Puts the tag into public mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableQt (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Disable QT. Puts the tag into private mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableQt (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Enables QT short range.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableQtShortRange (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Disable QT short range.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableQtShortRange (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Enables block permalocking over RF.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableBlockPermlock (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Disables block permalocking over RF.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableBlockPermlock (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Enables write wakeup mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxEnableWriteWakeupMode (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

/**

  Disables write wakeup mode.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxDisableWriteWakeupMode (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  );

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
  IN MONZAX_IO_PROTOCOL            *MonzaXIo,
  IN UINT8                         *Epc,
  IN UINTN                         EpcLen
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**

  Permanently locks the I2C device address.
  This only supported by Monza X 8K Dura.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written

**/
UINTN
EFIAPI
MonzaxLockI2cDeviceId (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo
  );

#endif
