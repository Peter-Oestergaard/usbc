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

int XID_::getInterface(uint8_t* interfaceCount)
{
  dataArray_[4] += 1;
  Serial.println("XID_::getInterface");
	*interfaceCount += 1; // uses 1
	XIDDescriptor xidInterface = {
		D_INTERFACE(pluggedInterface, 2, XID_INTERFACECLASS, XID_INTERFACESUBCLASS, XID_PROTOCOL_NONE),
		//D_XIDREPORT(descriptorSize),
		D_ENDPOINT(USB_ENDPOINT_IN(XID_EP_IN), USB_ENDPOINT_TYPE_INTERRUPT, 32, 0x04),
    D_ENDPOINT(USB_ENDPOINT_OUT(XID_EP_OUT), USB_ENDPOINT_TYPE_INTERRUPT, 32, 0x04)
	};
	return USBD_SendControl(0, &xidInterface, sizeof(xidInterface));
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

int XID_::getDescriptor(USBSetup &setup)
{
  dataArray_[2] += 1;
    //Device descriptor for duke, seems to work fine for Steel Battalion. Keep constant.
    USBD_SendControl(TRANSFER_PGM, &xid_dev_descriptor, sizeof(xid_dev_descriptor));
    return sizeof(xid_dev_descriptor);
}

bool XID_::setup(USBSetup& setup)
{
  dataArray_[1] += 1;
	//Serial.println("XID_::setup");

	return false;
}

XID_::XID_(int* dataArray) : PluggableUSBModule(2, 1, epType), dataBuffer(dataArray)//,
                   //rootNode(NULL), descriptorSize(0),
                   //protocol(1), idle(1)
{
  dataArray_ = dataArray;
  dataArray_[0] += 1;
	epType[0] = EP_TYPE_INTERRUPT_IN;
  epType[1] = EP_TYPE_INTERRUPT_OUT;
  memset(xid_out_data, 0x00, sizeof(xid_out_data));
  memset(xid_in_data, 0x00, sizeof(xid_in_data));
  xid_type = STEELBATTALION;
	int plug = PluggableUSB().plug(this);
  dataArray_[0] += 1;
  dataArray_[31] = plug;
}