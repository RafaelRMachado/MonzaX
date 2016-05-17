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

#include <Uefi.h>

#include <Protocol/MonzaXIo.h>

#include <Library/ReportStatusCodeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/MonzaXLib.h>

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpHex(
  IN UINT8  *Data,
  IN UINTN  Size
  );

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
    Print (L"%02x ", (UINTN)Data[Index]);
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
//    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
#ifdef DUMP_ASCII
    Print (L"        '");
    for ( Index2 = 0; Index2 < COLUME_SIZE; Index2++ ) {
      Print (L"%c", IsPrintable((Data + Index * COLUME_SIZE)[Index2]) ? (Data + Index * COLUME_SIZE)[Index2] : '.');
    }
    Print (L"'");
#endif
    Print (L"\n");
  }

  if (Left != 0) {
//    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
#ifdef DUMP_ASCII
    for ( Index2 = 0; Index2 < COLUME_SIZE - Left; Index2++ ) {
      Print (L"   ");
    }
    Print (L"        '");
    for ( Index2 = 0; Index2 < Left; Index2++ ) {
      Print (L"%c", IsPrintable((Data + Index * COLUME_SIZE)[Index2]) ? (Data + Index * COLUME_SIZE)[Index2] : '.');
    }
    Print (L"'");
#endif
    Print (L"\n");
  }
}

SHELL_PARAM_ITEM mParamList[] = {
  {L"-I",   TypeValue},
  {L"-?",   TypeFlag},
  {L"-h",   TypeFlag},
  {NULL,    TypeMax},
  };

/**
  Print application usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print (
    L"MonzaX App Version 0.1\n"
    L"Copyright (C) Intel Corp 2013. All rights reserved.\n"
    L"\n"
    );
  Print (
    L"MonzaX App in EFI Shell Environment.\n"
    L"\n"
    L"usage: MonzaXApp -I <TestIndex>\n"
    );

  Print (L"Monza X API Test\n");
  Print (L"-------------------------\n");
  Print (L" 1: Dump memory\n");
  Print (L" 2: Fill user memory with 0x00\n");
  Print (L" 3: Fill user memory with 0xFF\n");
  Print (L" 4: Fill user memory sequentially\n");
  Print (L" 5: Enable RF\n");
  Print (L" 6: Disable RF\n");
  Print (L" 7: Read model number\n");
  Print (L" 8: Chip test\n");
  Print (L" 9: Zero access password\n");
  Print (L"10: Zero kill password\n");
  Print (L"11: Lock access password\n");
  Print (L"12: Unlock access password\n");
  Print (L"13: Lock kill password\n");
  Print (L"14: Unlock kill password\n");
  Print (L"15: Lock EPC\n");
  Print (L"16: Unlock EPC\n");
  Print (L"17: Lock user memory\n");
  Print (L"18: Unlock user memory\n");
  Print (L"19: Kill tag\n");
  Print (L"20: Un-kill tag\n");
  Print (L"21: Enable QT\n");
  Print (L"22: Disable QT\n");
  Print (L"23: Enable QT short range\n");
  Print (L"24: Disable QT short range\n");
  Print (L"25: Set I2C device ID to 0x68\n");
  Print (L"26: Set I2C device ID to 0x6A\n");
  Print (L"27: Set I2C device ID to 0x6C\n");
  Print (L"28: Set I2C device ID to 0x6E\n");
  Print (L"29: Read EPC\n");
  Print (L"30: Read TID\n");
  Print (L"31: Unlock Block 0\n");
  Print (L"32: Permalock Block 0\n");
  Print (L"33: Permalock User memory\n");
  Print (L"34: Enable Write Wakeup Mode (WWU)\n");
  Print (L"35: Disable Write Wakeup Mode (WWU)\n");
  Print (L"99: Exit\n");
}

/**
  Check test result.

  @param Result     Result data
  @param Requested  Requested data
**/
VOID
CheckResult (
  IN UINTN  Result,
  IN UINTN  Requested
  )
{
  if (Result < Requested) {
    Print (L"Command failed. Are you accessing the chip through RF?\n");
  }
}

/**
  Print MonzaX information.

  @param MonzaXIo  MonzaX IO instance
**/
VOID
PrintInfo (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  MONZAX_INFO  MonzaxInfo;

  MonzaxInfo.Revision = MONZAX_INFO_REVISION;
  MonzaxInfo.Length   = sizeof(MONZAX_INFO);

  MonzaXIo->GetInfo (MonzaXIo, &MonzaxInfo);
  if (MonzaxInfo.ChipModelType == MonzaX2KDura) {
    Print(L"\nMonzaX 2K Dura");
  } else {
    Print(L"\nMonzaX 8K Dura");
  }
  Print (L" detected at I2C address 0x%02X\n\n", MonzaxInfo.I2cDeviceId);
}

/**
  Write entried user memory.

  @param MonzaXIo  MonzaX IO instance
  @param Data      The data used to write to entired user memory.

  @return  The number of bytes written
**/
UINTN
MonzaXWriteEntireUserMemory (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINT8                  Data
  )
{
  UINTN      UserMemSize;
  UINT8      *Buffer;
  UINTN      Count;

  UserMemSize = MonzaxGetBankSize (MonzaXIo, MonzaXMemoryBankUser);

  Buffer = AllocatePool (UserMemSize);
  if (Buffer == NULL) {
    return 0;
  }
  SetMem (Buffer, UserMemSize, Data);
  Count = MonzaxWriteBank (MonzaXIo, MonzaXMemoryBankUser, 0, Buffer, UserMemSize);
  CheckResult (Count, UserMemSize);
  FreePool (Buffer);

  return Count;
}

/**
  Write entried user memory with sequence.

  @param MonzaXIo  MonzaX IO instance

  @return  The number of bytes written
**/
UINTN
MonzaXWriteEntireUserMemorySeq (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  UINTN      Index;
  UINTN      UserMemSize;
  UINT8      *Buffer;
  UINTN      Count;

  UserMemSize = MonzaxGetBankSize (MonzaXIo, MonzaXMemoryBankUser);

  Buffer = AllocatePool (UserMemSize);
  if (Buffer == NULL) {
    return 0;
  }
  for (Index = 0; Index < UserMemSize; Index++) {
    Buffer[Index] = (UINT8)(Index % 0x100);
  }
  Count = MonzaxWriteBank (MonzaXIo, MonzaXMemoryBankUser, 0, Buffer, UserMemSize);
  CheckResult (Count, UserMemSize);
  FreePool (Buffer);

  return Count;
}

/**
  Dump full MonzaX memory.

  @param MonzaXIo  MonzaX IO instance
**/
VOID
MonzaXDumpMemory (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  UINTN Count;
  UINTN MemSize;
  UINT8 Buffer[MONZAX_SIZE_BYTES_USER_8K];

  // Dump all memory banks

  // MonzaXMemoryBankUser
  Print (L"User\n");
  MemSize = MonzaxGetBankSize (MonzaXIo, MonzaXMemoryBankUser);
  Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankUser, 0, Buffer, MemSize);
  CheckResult (Count, MemSize);
  Print (L"User Buffer:\n");
  InternalDumpHex (Buffer, Count);

  // TID
  Print (L"TID\n");
  MemSize = MonzaxGetBankSize (MonzaXIo, MonzaXMemoryBankTid);
  Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankTid, 0, Buffer, MemSize);
  CheckResult (Count, MemSize);
  Print (L"TID Buffer:\n");
  InternalDumpHex (Buffer, Count);

  // EPC
  Print (L"EPC\n");
  MemSize = MonzaxGetBankSize (MonzaXIo, MonzaXMemoryBankEpc);
  Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankEpc, 0, Buffer, MemSize);
  CheckResult(Count, MemSize);
  Print (L"EPC Buffer:\n");
  InternalDumpHex (Buffer, Count);

  // Reserved
  Print (L"Reserved\n");
  MemSize = MonzaxGetBankSize (MonzaXIo, MonzaXMemoryBankReserved);
  Count = MonzaxReadBank (MonzaXIo, MonzaXMemoryBankReserved, 0, Buffer, MemSize);
  CheckResult(Count, MemSize);
  Print (L"Reserved Buffer:\n");
  InternalDumpHex (Buffer, Count);

  return ;
}

/**
  Read EPC data.

  @param MonzaXIo   MonzaX IO instance

  @return  The number of bytes read
**/
UINTN
MonzaXReadEpc (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  UINT8       Buffer[MONZAX_SIZE_BYTES_EPC];
  UINTN       Count;

  Count = MonzaxGetEpc (MonzaXIo, Buffer, sizeof(Buffer));
  Print (L"EPC Buffer:\n");
  InternalDumpHex (Buffer, Count);
  return Count;
}

/**
  Read TID data.

  @param MonzaXIo   MonzaX IO instance

  @return  The number of bytes read
**/
UINTN
MonzaXReadTid (
  IN MONZAX_IO_PROTOCOL *MonzaXIo
  )
{
  UINT8       Buffer[MONZAX_SIZE_BYTES_TID];
  UINTN       Count;

  Count = MonzaxGetTid (MonzaXIo, Buffer, sizeof(Buffer));
  Print(L"TID Buffer:\n");
  InternalDumpHex(Buffer, Count);
  return Count;
}

/**
  Run APP test.

  @param MonzaXIo   MonzaX IO instance
  @param TestIndex  Index of test case
**/
VOID
MonzaXAppTest (
  IN MONZAX_IO_PROTOCOL     *MonzaXIo,
  IN UINTN                  TestIndex
  )
{
  UINTN                  Count;
  UINT8                  Result;
  UINT16                 ModelNum;

  switch (TestIndex) {
  case 1:
    MonzaXDumpMemory (MonzaXIo);
    break;
  case 2:
    MonzaXWriteEntireUserMemory (MonzaXIo, 0x00);
    break;
  case 3:
    MonzaXWriteEntireUserMemory (MonzaXIo, 0xFF);
    break;
  case 4:
    MonzaXWriteEntireUserMemorySeq (MonzaXIo);
    break;
  case 5:
    Count = MonzaxEnableRfDci (MonzaXIo);
    CheckResult (Count, 1);
    Count = MonzaxEnableRfPort1 (MonzaXIo);
    CheckResult (Count, 1);
    Count = MonzaxEnableRfPort2 (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 6:
    Count = MonzaxDisableRfDci (MonzaXIo);
    CheckResult (Count, 1);
    Count = MonzaxDisableRfPort1 (MonzaXIo);
    CheckResult (Count, 1);
    Count = MonzaxDisableRfPort2 (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 7:
    ModelNum = MonzaxReadModelNumber (MonzaXIo);
    Print (L"Chip model number : 0x%03X\n\n", ModelNum);
    break;
  case 8:
    Result = MonzaxChipTest (MonzaXIo);
    if (Result == 0) {
      Print (L"Chip is alive!\n\n");
    } else {
      Print (L"Chip did not respond\n\n");
    }
    break;
  case 9:
    Count = MonzaxSetAccessPw (MonzaXIo, 0x00000000);
    CheckResult (Count, 4);
    break;
  case 10:
    Count = MonzaxSetKillPw (MonzaXIo, 0x00000000);
    CheckResult (Count, 4);
    break;
  case 11:
    Count = MonzaxLockAccessPw (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 12:
    Count = MonzaxUnlockAccessPw (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 13:
    Count = MonzaxLockKillPw (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 14:
    Count = MonzaxUnlockKillPw (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 15:
    Count = MonzaxLockEpc (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 16:
    Count = MonzaxUnlockEpc (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 17:
    Count = MonzaxLockUser (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 18:
    Count = MonzaxUnlockUser (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 19:
    Count = MonzaxKillTag (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 20:
    Count = MonzaxUnkillTag (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 21:
    Count = MonzaxEnableQt (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 22:
    Count = MonzaxDisableQt (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 23:
    Count = MonzaxEnableQtShortRange (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 24:
    Count = MonzaxDisableQtShortRange (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 25:
    Count = MonzaxSetI2cDeviceId (MonzaXIo, 0x68);
    CheckResult (Count, 1);
    break;
  case 26:
    Count = MonzaxSetI2cDeviceId (MonzaXIo, 0x6A);
    CheckResult (Count, 1);
    break;
  case 27:
    Count = MonzaxSetI2cDeviceId (MonzaXIo, 0x6C);
    CheckResult (Count, 1);
    break;
  case 28:
    Count = MonzaxSetI2cDeviceId (MonzaXIo, 0x6E);
    CheckResult (Count, 1);
    break;
  case 29:
    Count = MonzaXReadEpc (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 30:
    Count = MonzaXReadTid (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 31:
    Count = MonzaxBlockUnlock (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 32:
    Count = MonzaxBlockPermalock (MonzaXIo, 0);
    CheckResult (Count, 1);
    break;
  case 33:
    Count = MonzaxLockUser (MonzaXIo, 1);
    CheckResult (Count, 1);
    break;
  case 34:
    Count = MonzaxEnableWriteWakeupMode (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 35:
    Count = MonzaxDisableWriteWakeupMode (MonzaXIo);
    CheckResult (Count, 1);
    break;
  case 99:
    break;
  default:
    Print (L"Invalid option\n");
    break;
  }
}

/**
  MonzaX APP entrypoint.

  @param[in]  ImageHandle     The image handle.
  @param[in]  SystemTable     The system table.

  @retval EFI_SUCCESS            Command completed successfully.
**/
EFI_STATUS
EFIAPI
MonzaXAppEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS             Status;
  MONZAX_IO_PROTOCOL     *MonzaXIo;
  LIST_ENTRY             *ParamPackage;
  CHAR16                 *IndexStr;
  UINTN                  TestIndex;

  Status = gBS->LocateProtocol (
                  &gMonzaXIoProtocolGuid,
                  NULL,
                  (VOID **)&MonzaXIo
                  );
  if (EFI_ERROR (Status)) {
    Print (L"MonzaXIo - %r\n", Status);
    return Status;
  }

  PrintInfo (MonzaXIo);

  Status = ShellCommandLineParse (mParamList, &ParamPackage, NULL, TRUE);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Incorrect command line.\n");
    return Status;
  }

  if (ParamPackage == NULL ||
     ShellCommandLineGetFlag(ParamPackage, L"-?") ||
     ShellCommandLineGetFlag(ParamPackage, L"-h")) {
    PrintUsage ();
    return EFI_SUCCESS;
  }

  IndexStr = (CHAR16 *)ShellCommandLineGetValue(ParamPackage, L"-I");
  if (IndexStr == NULL) {
    PrintUsage ();
    return EFI_SUCCESS;
  }

  TestIndex = StrDecimalToUintn (IndexStr);

  MonzaXAppTest (MonzaXIo, TestIndex);

  return EFI_SUCCESS;
}
