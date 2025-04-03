// Copyright 2021, Ryan Wendland, ogx360
// SPDX-License-Identifier: GPL-3.0-or-later

//#include <Arduino.h>

#include "main.h"
#include "usbd_xid.h"
//#include "usbh/usbh_xinput.h"

uint8_t player_id;
XID_ usbd_xid;
usbd_controller_t usbd_c[MAX_GAMEPADS];

void setup() {
  Serial.begin(115200);
  Serial.println("setup");

  pinMode(ARDUINO_LED_PIN, OUTPUT);
  pinMode(PLAYER_ID1_PIN, INPUT_PULLUP);
  pinMode(PLAYER_ID2_PIN, INPUT_PULLUP);
  digitalWrite(ARDUINO_LED_PIN, HIGH);

  memset(usbd_c, 0x00, sizeof(usbd_controller_t) * MAX_GAMEPADS);

  for (uint8_t i = 0; i < MAX_GAMEPADS; i++) {
    usbd_c[i].type = DUKE;
    usbd_c[i].duke.in.bLength = sizeof(usbd_duke_in_t);
    usbd_c[i].duke.out.bLength = sizeof(usbd_duke_out_t);

    usbd_c[i].sb.in.bLength = sizeof(usbd_sbattalion_in_t);
    usbd_c[i].sb.out.bLength = sizeof(usbd_sbattalion_out_t);
  }
}

void loop() {
  static uint32_t poll_timer = 0;
  if (millis() - poll_timer > 4) {

    usbd_xid.sendReport(&usbd_c[0].sb.in, sizeof(usbd_sbattalion_in_t));
    usbd_xid.getReport(&usbd_c[0].sb.out, sizeof(usbd_sbattalion_out_t));


    poll_timer = millis();
  }
}
