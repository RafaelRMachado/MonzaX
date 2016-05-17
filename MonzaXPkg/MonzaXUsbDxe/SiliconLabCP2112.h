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

#ifndef _SILICONLIB_CP2112_H_
#define _SILICONLIB_CP2112_H_

#define CP2112_VID 0x10C4
#define CP2112_PID 0xEA90

#pragma pack(1)

//
// Device Configuration (Feature Request)
//
#define CP2112_RESET_DEVICE                  0x01
#define CP2112_GET_SET_GPIO_CONFIGURATION    0x02
#define CP2112_GET_GPIO                      0x03
#define CP2112_SET_GPIO                      0x04
#define CP2112_GET_VERSION_INFORMATION       0x05
#define CP2112_GET_SET_SMBUS_CONFIGURATION   0x06

//
// Data Transfer (Interrupt Transfer)
//
#define CP2112_DATA_READ_REQUEST             0x10
#define CP2112_DATA_WRITE_READ_REQUEST       0x11
#define CP2112_DATA_READ_FORCE_SEND          0x12
#define CP2112_DATA_READ_RESPONSE            0x13
#define CP2112_DATA_WRITE                    0x14
#define CP2112_TRANSFER_STATUS_REQUEST       0x15
#define CP2112_TRANSFER_STATUS_RESPONSE      0x16
#define CP2112_CANCEL_TRANSFER               0x17

//
// USB Customization (Feature Requests)
//
#define CP2112_GET_SET_LOCK_BYTE             0x20
#define CP2112_GET_SET_USB_CONFIGURATION     0x21
#define CP2112_GET_SET_MANUFACTURING_STRING  0x22
#define CP2112_GET_SET_PRODUCT_STRING        0x23
#define CP2112_GET_SET_SERIAL_STRING         0x24

// NOTE: All below data structure is BIG ENDIEN.

typedef struct {
  UINT8    Command;
  UINT8    SlaveAddress;
  UINT16   Length;
} CP2112_DATA_READ_REQUEST_STRUCT;

typedef struct {
  UINT8    Command;
  UINT8    SlaveAddress;
  UINT16   Length;
  UINT8    TargetAddressLength;
  UINT8    TargetAddress[16];
} CP2112_DATA_WRITE_READ_REQUEST_STRUCT;

typedef struct {
  UINT8    Command;
  UINT16   Length;
} CP2112_DATA_READ_FORCE_SEND_STRUCT;

#define CP2112_DATA_READ_RESPONSE_STATUS_IDLE      0
#define CP2112_DATA_READ_RESPONSE_STATUS_BUSY      1
#define CP2112_DATA_READ_RESPONSE_STATUS_COMPLETE  2
#define CP2112_DATA_READ_RESPONSE_STATUS_ERROR     3
typedef struct {
  UINT8    Command;
  UINT8    Status;
  UINT8    Length;
  UINT8    Data[61];
} CP2112_DATA_READ_RESPONSE_STRUCT;

typedef struct {
  UINT8    Command;
  UINT8    SlaveAddress;
  UINT8    Length;
  UINT8    Data[61];
} CP2112_DATA_WRITE_STRUCT;

#define CP2112_TRANSFER_STATUS_REQUEST_SMBUS_TRANSFER_STATUS     1
typedef struct {
  UINT8    Command;
  UINT8    Request;
  UINT8    Reserved[5];
} CP2112_TRANSFER_STATUS_REQUEST_STRUCT;

#define CP2112_TRANSFER_STATUS_RESPONSE_STATUS0_IDLE      0
#define CP2112_TRANSFER_STATUS_RESPONSE_STATUS0_BUSY      1
#define CP2112_TRANSFER_STATUS_RESPONSE_STATUS0_COMPLETE  2
#define CP2112_TRANSFER_STATUS_RESPONSE_STATUS0_ERROR     3
#define CP2112_TRANSFER_STATUS_RESPONSE_BUSY_STATUS1_ADDRESS_ACKED     0
#define CP2112_TRANSFER_STATUS_RESPONSE_BUSY_STATUS1_ADDRESS_NACKED    1
#define CP2112_TRANSFER_STATUS_RESPONSE_BUSY_STATUS1_DATA_READ         2
#define CP2112_TRANSFER_STATUS_RESPONSE_BUSY_STATUS1_DATA_WRITE        3
#define CP2112_TRANSFER_STATUS_RESPONSE_ERROR_STATUS1_TIMEOUT_ADDRESS_NACKED     0
#define CP2112_TRANSFER_STATUS_RESPONSE_ERROR_STATUS1_TIMEOUT_BUS_NOT_FREE       1
#define CP2112_TRANSFER_STATUS_RESPONSE_ERROR_STATUS1_ARBITRATION_LOST           2
#define CP2112_TRANSFER_STATUS_RESPONSE_ERROR_STATUS1_READ_INCOMPLETE            3
#define CP2112_TRANSFER_STATUS_RESPONSE_ERROR_STATUS1_WRITE_INCOMPLETE           4
#define CP2112_TRANSFER_STATUS_RESPONSE_ERROR_STATUS1_SUCCESS                    5
typedef struct {
  UINT8    Command;
  UINT8    Status0;
  UINT8    Status1;
  UINT16   Status2; // Number of retries before completing, being canceled, or timing out
  UINT16   Status3; // Number of received bytes
} CP2112_TRANSFER_STATUS_RESPONSE_STRUCT;

#define CP2112_CANCEL_TRANSFER_CANCEL     1
typedef struct {
  UINT8    Command;
  UINT8    Cancel;
  UINT8    Reserved[5];
} CP2112_CANCEL_TRANSFER_STRUCT;

#pragma pack()


#endif
