#include <Arduino.h>
#include <USB/PluggableUSB.h>

#define TRANSFER_PGM 0x80

#define XID_INTERFACECLASS 88
#define XID_INTERFACESUBCLASS 66

#define XID_EP_IN 0x82
#define XID_EP_OUT 0x01

#define XID_GET_REPORT        0x01
#define XID_GET_IDLE          0x02
#define XID_GET_PROTOCOL      0x03
#define XID_SET_REPORT        0x09
#define XID_SET_IDLE          0x0A
#define XID_SET_PROTOCOL      0x0B

#define XID_REPORT_DESCRIPTOR_TYPE      0x22

#define XID_SUBCLASS_NONE 0

#define XID_PROTOCOL_NONE 0

static const DeviceDescriptor xid_dev_descriptor PROGMEM =
    D_DEVICE(0x00, 0x00, 0x00, 8, USB_VID, USB_PID, 0x0100, 0, 0, 0, 1);

typedef struct 
{
  InterfaceDescriptor xid;
  EndpointDescriptor  in;
  EndpointDescriptor  out;
} XIDDescriptor;

class XIDSubDescriptor {
public:
  XIDSubDescriptor *next = NULL;
  XIDSubDescriptor(const void *d, const uint16_t l) : data(d), length(l) { }

  const void* data;
  const uint16_t length;
};

typedef enum
{
    DISCONNECTED = 0,
    DUKE,
    STEELBATTALION
} xid_type_t;

class XID_ : public PluggableUSBModule
{
public:
  XID_(int* dataArray);
  int begin(void);
  int SendReport(uint8_t id, const void* data, int len);
  //int getReport(void *data, int len);
  int getReport();
  void AppendDescriptor(XIDSubDescriptor* node);

  int* dataBuffer;

protected:
  int getInterface(uint8_t* interfaceCount);
  int getDescriptor(USBSetup& setup);
  bool setup(USBSetup& setup);
  uint8_t getShortName(char *name);

private:
    xid_type_t xid_type;
    uint32_t epType[2];
    uint8_t xid_in_data[32];
    uint8_t xid_out_data[32];
    uint32_t xid_out_expired;
};

// Replacement for global singleton.
// This function prevents static-initialization-order-fiasco
// https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
XID_& XID(int* dataArray = nullptr);

#define D_XIDREPORT(length) { 9, 0x21, 0x01, 0x01, 0, 1, 0x22, lowByte(length), highByte(length) }