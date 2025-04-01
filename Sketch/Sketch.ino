#include "XID.h"

int myData[32] = {0};
XID_ usbd_xid(myData);

//usbd_steelbattalion_t sb;

void setup() {
  Serial.begin(9600);
  //Serial.println(USBCON);
  //USBDevice.configured();
  //int succ = USBDevice.attach();
  //Serial.println(succ);
}

void loop() {
  //usbd_xid.getReport();
  for (int i = 0; i < 32; i++) {
    Serial.print(myData[i]);
    Serial.print(" ");
  }
  Serial.println();

  //usbd_xid.getReport();

  delay(1000);
}
