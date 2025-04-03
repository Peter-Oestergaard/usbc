// Copyright 2021, Ryan Wendland, ogx360
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbd_xid.h"

#define TRANSFER_PGM 0x80

#define ENABLE_USBD_XID_DEBUG
#ifdef ENABLE_USBD_XID_DEBUG
#define USBD_XID_DEBUG(a) Serial.print(F(a))
#else
#define USBD_XID_DEBUG(...)
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

XID_ &XID()
{
    static XID_ obj;
    return obj;
}

int XID_::getInterface(uint8_t *interfaceCount)
{
    Serial.println("XID_::getInterface");
    *interfaceCount += 1;

    XIDDescriptor xid_interface = {
        D_INTERFACE(pluggedInterface, 2, XID_INTERFACECLASS, XID_INTERFACESUBCLASS, 0),
        D_ENDPOINT(USB_ENDPOINT_IN(XID_EP_IN), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x04),
        D_ENDPOINT(USB_ENDPOINT_OUT(XID_EP_OUT), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x04)};

    return USBD_SendControl(0, &xid_interface, sizeof(xid_interface));
}

int XID_::getDescriptor(USBSetup &setup)
{
    Serial.println("XID_::getDescriptor");
    //Device descriptor for duke, seems to work fine for Steel Battalion. Keep constant.
    USBD_SendControl(TRANSFER_PGM, &xid_dev_descriptor, MIN(sizeof(xid_dev_descriptor), setup.wLength));
    //return sizeof(xid_dev_descriptor);
    return MIN(sizeof(xid_dev_descriptor), setup.wLength);
}

int XID_::sendReport(const void *data, int len)
{
    int capped_len = min((unsigned int)len, sizeof(xid_in_data));
    if (memcmp(xid_in_data, data, capped_len) != 0)
    {
        //Update local copy, then send
        if (USBD_Send(XID_EP_IN | TRANSFER_RELEASE, data, capped_len) == len)
            memcpy(xid_in_data, data, capped_len);
    }
    return len;
}

int XID_::getReport(void *data, int len)
{
    int capped_len = min((uint32_t)len, sizeof(xid_out_data));
    uint8_t r[capped_len];// = {0};
    for (int i = 0; i < capped_len; i++ ) { r[i] = 0; }
    if (USBD_Recv(XID_EP_OUT | TRANSFER_RELEASE, r, capped_len) == capped_len)
    {
        USBD_XID_DEBUG("USBD XID: GOT HID REPORT OUT FROM ENDPOINT\n");
        memcpy(xid_out_data, r, capped_len);
        memcpy(data, r, capped_len);
        xid_out_expired = millis();
        return capped_len;
    }
    //No new data on interrupt pipe, if its been a while since last update,
    //Treat it as expired. Prevents rumble locking on old values.
    if(millis() - xid_out_expired > 500)
    {
        memset(data, 0x00, capped_len);
        return 0;
    }
    //No new data on interrupt pipe, return previous data
    memcpy(data, xid_out_data, capped_len);
    return xid_out_data[1];
}

bool XID_::setup(USBSetup &setup)
{
    if (pluggedInterface != setup.wIndex)
    {
        return false;
    }

    uint8_t request = setup.bRequest;
    uint8_t requestType = setup.bmRequestType;
    uint16_t wValue = (setup.wValueH << 8) | (setup.wValueL & 0xFF);

    if (requestType == (REQUEST_DEVICETOHOST | REQUEST_VENDOR | REQUEST_INTERFACE))
    {
        if (request == 0x06 && wValue == 0x4200)
        {
            USBD_XID_DEBUG("USBD XID: SENDING XID DESCRIPTOR\n");
            if (xid_type == DUKE)
            {
                USBD_SendControl(TRANSFER_PGM, DUKE_DESC_XID, MIN(sizeof(DUKE_DESC_XID), setup.wLength));
            }
            else if (xid_type == STEELBATTALION)
            {
                USBD_SendControl(TRANSFER_PGM, BATTALION_DESC_XID, MIN(sizeof(BATTALION_DESC_XID), setup.wLength));
            }
            return true;
        }
        if (request == 0x01 && wValue == 0x0100)
        {
            USBD_XID_DEBUG("USBD XID: SENDING XID CAPABILITIES IN\n");
            USBD_SendControl(TRANSFER_PGM, DUKE_CAPABILITIES_IN, MIN(sizeof(DUKE_CAPABILITIES_IN), setup.wLength));
            return true;
        }
        if (request == 0x01 && wValue == 0x0200)
        {
            USBD_XID_DEBUG("USBD XID: SENDING XID CAPABILITIES OUT\n");
            USBD_SendControl(TRANSFER_PGM, DUKE_CAPABILITIES_OUT, MIN(sizeof(DUKE_CAPABILITIES_OUT), setup.wLength));
            return true;
        }
    }

    if (requestType == (REQUEST_DEVICETOHOST | REQUEST_CLASS | REQUEST_INTERFACE))
    {
        if (request == HID_GET_REPORT && setup.wValueH == HID_REPORT_TYPE_INPUT)
        {
            USBD_XID_DEBUG("USBD XID: SENDING HID REPORT IN\n");
            USBD_SendControl(0, xid_in_data, MIN(sizeof(xid_in_data), setup.wLength));
            return true;
        }
    }

    if (requestType == (REQUEST_HOSTTODEVICE | REQUEST_CLASS | REQUEST_INTERFACE))
    {
        if (request == HID_SET_REPORT && setup.wValueH == HID_REPORT_TYPE_OUTPUT)
        {
            USBD_XID_DEBUG("USBD XID: GETTING HID REPORT OUT\n");
            uint16_t length = min(sizeof(xid_out_data), setup.wLength);
            USBD_RecvControl(xid_out_data, length);
            xid_out_expired = millis();
            return true;
        }
    }

    USBD_XID_DEBUG("USBD XID: STALL\n");
    Serial.println(requestType, HEX);
    Serial.println(request, HEX);
    Serial.println(wValue, HEX);
    return false;
}

void XID_::setType(xid_type_t type)
{
    if (xid_type == type)
    {
        return;
    }

    xid_type = type;
    delay(10);
    return;
}

xid_type_t XID_::getType(void)
{
    return xid_type;
}

XID_::XID_(void) : PluggableUSBModule(2, 1, epType)
{
    epType[0] = EP_TYPE_INTERRUPT_IN;
    epType[1] = EP_TYPE_INTERRUPT_OUT;
    memset(xid_out_data, 0x00, sizeof(xid_out_data));
    memset(xid_in_data, 0x00, sizeof(xid_in_data));
    xid_type = STEELBATTALION;
    PluggableUSB().plug(this);
}

int XID_::begin(void)
{
    return 0;
}
