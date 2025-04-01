#include "XID.h"

int* dataArray_;

XID_& XID(int* dataArray)
{
	static XID_ obj(dataArray);
	return obj;
}

int XID_::begin(void) {
  dataArray_[6] += 1;
  return 0;
}

uint8_t XID_::getShortName(char *name) {
  dataArray_[5] += 1;
  return 0;
}

const uint8_t p1[] = {
  //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    0x09, 0x04, 0x00, 0x00, 0x01, 0x09, 0x00, 0x00, 0x00, 0x07, 0x05, 0x81, 0x03, 0x01, 0x00, 0xff
};
const uint8_t p2[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const uint8_t p3[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const uint8_t p4[] = {
  0x00
};

#ifndef USB_EP_SIZE
#define USB_EP_SIZE 1
#endif

// Configuraion response fragments from SBC controller
// 09021900010100a0 3209040000010900 0000070581030100 ff
// 09 02 19 00 01 01 00 a0
//                         32  09 04 00 00 01 09 00
//                                                  00 00  07 05 81 03 01 00
//                                                                           ff
//                              00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

// Current version response (as single packet)
// 09 02 19 00 01 01 00 80 fa  09 04 00 00 01 58 42 00 00  07 05 81 03 01 00 ff
// Diffs:               ^^ ^^                 ^^ ^^
//                       b  b                  b  b               b     w     b
//                       m  M                  I  I               E     M     I
//                       A  a                  n  n               n     a     n
//                       t  x                  t  t               d     x     t
//                       t  P                  e  e               P     P     e
//                       r  o                  r  r               o     a     r
//                       i  w                  f  f               i     c     v
//                       b  e                  a  a               n     k     a
//                       u  r                  c  c               t     e     l
//                       t                     e  e               A     t
//                       e                     C  S               d     S
//                       s                     l  u               d     i
//                                             a  b               r     z
//                                             s  C               e     e
//                                             s  l               s
//                                                a               s
//                                                s
//                                                s

int XID_::getInterface(uint8_t *interfaceCount)
{
    Serial.print("XID_::getInterface. Count: ");
    Serial.println(*interfaceCount);
    *interfaceCount += 1;

    XIDDescriptor xid_interface = {
        D_INTERFACE(pluggedInterface, 1, XID_INTERFACECLASS, XID_INTERFACESUBCLASS, 0),
        //D_ENDPOINT(USB_ENDPOINT_IN(XID_EP_IN), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x04),
        //D_ENDPOINT(USB_ENDPOINT_OUT(XID_EP_OUT), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x04)};
        D_ENDPOINT(0x81, USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0xff)};

    return USBD_SendControl(0, &xid_interface, sizeof(xid_interface));
    //int total = 0;
    //total += USBD_SendControl(0, p1, sizeof(p1));
    //total += USBD_Send(0, 0, 1);;
    //total += USBD_SendControl(0, p2, 8);
    //total += USBD_SendControl(0, p3, 8);
    //total += USBD_SendControl(0, p4, 1);
    //return total;
}
/*
int XID_::sendReport(const void *data, int len)
{
    int capped_len = min((unsigned int)len, sizeof(xid_in_data));
    if (memcmp(xid_in_data, data, capped_len) != 0)
    {
        //Update local copy, then send
        if (USB_Send(XID_EP_IN | TRANSFER_RELEASE, data, capped_len) == len)
            memcpy(xid_in_data, data, capped_len);
    }
    return len;
}
*/
//int XID_::getReport(void *data, int len)
int XID_::getReport()
{
  dataArray_[3] += 1;
  //Serial.println("XID_::getReport");
    int capped_len = 10;//min((uint32_t)len, sizeof(xid_out_data));
    uint8_t r[10] = {0};
    int recb = USBD_Recv(XID_EP_IN);
    Serial.print("Got: ");
    Serial.println(recb);
    if (recb == capped_len)
    {
        //Serial.println("USBD XID: GOT HID REPORT OUT FROM ENDPOINT");
        //memcpy(xid_out_data, r, capped_len);
        //memcpy(data, r, capped_len);
        //xid_out_expired = millis();
        return capped_len;
    }
    /*
    //No new data on interrupt pipe, if its been a while since last update,
    //Treat it as expired. Prevents rumble locking on old values.
    if(millis() - xid_out_expired > 500)
    {
        memset(data, 0x00, capped_len);
        return 0;
    }
    */
    //No new data on interrupt pipe, return previous data
    //memcpy(data, xid_out_data, capped_len);
    return 0;//xid_out_data[1];
}
//const uint8_t test_descriptor[] = {
//  0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 0x08
//};

int XID_::getDescriptor(USBSetup &setup)
{
  dataArray_[2] += 1;
    //Device descriptor for duke, seems to work fine for Steel Battalion. Keep constant.
    //USBD_SendControl(TRANSFER_PGM, &xid_dev_descriptor, sizeof(xid_dev_descriptor));
    USBD_SendControl(TRANSFER_PGM, &xid_dev_descriptor, setup.wLength);
    //USBD_SendControl(TRANSFER_PGM, test_descriptor, sizeof(test_descriptor));
    //return sizeof(test_descriptor);
    return setup.wLength;
}

bool XID_::setup(USBSetup& setup)
{
  dataArray_[1] += 1;
	//Serial.println("XID_::setup");

	return false;
}

XID_::XID_(int* dataArray) : PluggableUSBModule(1, 1, epType), dataBuffer(dataArray)//,
                   //rootNode(NULL), descriptorSize(0),
                   //protocol(1), idle(1)
{
  dataArray_ = dataArray;
  dataArray_[0] += 1;
	epType[0] = EP_TYPE_INTERRUPT_IN;
  //epType[1] = EP_TYPE_INTERRUPT_OUT;
  //memset(xid_out_data, 0x00, sizeof(xid_out_data));
  //memset(xid_in_data, 0x00, sizeof(xid_in_data));
  //xid_type = STEELBATTALION;
	int plug = PluggableUSB().plug(this);
  dataArray_[0] += 1;
  dataArray_[31] = plug;
}