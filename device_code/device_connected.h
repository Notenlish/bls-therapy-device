#pragma once

void enterConnectedMode(TherapyDevice::DeviceMode &device_mode, WiFiServer &server) {
  device_mode = TherapyDevice::DeviceMode::Connected;
  // TODO: status led must be lighting green.
  // TODO: actually write this.

  server.begin();
  Serial.println("Started server at port 80; In Connected mode.");
}