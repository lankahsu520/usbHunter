/***************************************************************************
 * Copyright (C) 2017 - 2020, Lanka Hsu, <lankahsu@gmail.com>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#ifndef __USB_HUNTER_H__
#define __USB_HUNTER_H__

//******************************************************************************
//** include **
//******************************************************************************
#include "utilx9.h"

#define TAG "usbHunter"

//******************************************************************************
//** feature **
//******************************************************************************
//#define USE_UV
//#define USE_ASYNC_CREATE


//******************************************************************************
//** define **
//******************************************************************************
#if (1) // any
#define VENDOR_ID LIBUSB_HOTPLUG_MATCH_ANY
#define PRODUCT_ID LIBUSB_HOTPLUG_MATCH_ANY
#endif

#if (0)
#define VENDOR_ID 0x10C4
#define PRODUCT_ID 0xEA60
#endif

//******************************************************************************
//** function **
//******************************************************************************
int usb_hotplug_cb(struct libusb_context *ctx, struct libusb_device *usb_dev, libusb_hotplug_event event, void *user_data);


#endif
