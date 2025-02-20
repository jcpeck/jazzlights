#ifndef JAZZLIGHTS_NETWORK_ARDUINO_ETHERNET_H
#define JAZZLIGHTS_NETWORK_ARDUINO_ETHERNET_H

#ifndef CORE2AWS_ETHERNET
#define CORE2AWS_ETHERNET 0
#endif  // CORE2AWS_ETHERNET

#if CORE2AWS_ETHERNET && !defined(JAZZLIGHTS_ARDUINO_ETHERNET)
#define JAZZLIGHTS_ARDUINO_ETHERNET 1
#endif

#ifndef JAZZLIGHTS_ARDUINO_ETHERNET
#define JAZZLIGHTS_ARDUINO_ETHERNET 0
#endif  // JAZZLIGHTS_ARDUINO_ETHERNET

#if JAZZLIGHTS_ARDUINO_ETHERNET

#include <Ethernet.h>
#include <SPI.h>

#include "jazzlights/network.h"

namespace jazzlights {

class ArduinoEthernetNetwork : public UdpNetwork {
 public:
  explicit ArduinoEthernetNetwork(NetworkDeviceId localDeviceId);

  NetworkStatus update(NetworkStatus status, Milliseconds currentTime) override;
  int recv(void* buf, size_t bufsize, std::string* details) override;
  void send(void* buf, size_t bufsize) override;
  NetworkDeviceId getLocalDeviceId() override { return localDeviceId_; }
  const char* networkName() const override { return "ArduinoEthernet"; }
  std::string getStatusStr(Milliseconds currentTime) const override;

 private:
  NetworkDeviceId localDeviceId_;
  uint16_t port_ = DEFAULT_UDP_PORT;
  const char* mcastAddr_ = DEFAULT_MULTICAST_ADDR;
  EthernetUDP udp_;
};

}  // namespace jazzlights

#endif  // JAZZLIGHTS_ARDUINO_ETHERNET

#endif  // JAZZLIGHTS_NETWORK_ARDUINO_ETHERNET_H
