#include <stdlib.h>
#include "tgloader.hpp"
#include "extras.h"

using namespace jazzlights;

Player player;
UnixUdpNetwork network;
const char* sysinfo();
char version[256] = {};

extern "C" const char* call(const char* cmd) {
  if (!strcmp(cmd, "shutdown")) {
    if (!system("shutdown -h now")) {
      return "msg shutting down...";
    }
    return "! failed to shut down";
  }
  if (!strcmp(cmd, "sysinfo?")) {
    return sysinfo();
  }
  return player.command(cmd);
}

extern "C" void run(bool verbose, const char* ver, const char* cfgfile) {
  if (verbose) {
    enableVerboseOutput();    
  }

  snprintf(version, sizeof(version) - 1, "%s_%s", ver, BOOT_MESSAGE);
  info("My %s", sysinfo());
  player.setBasePrecedence(20000);
  player.setPrecedenceGain(5000);
  load(cfgfile, player);
  player.connect(&network);
  player.begin(timeMillis());
  for (;;) {
    player.render(timeMillis());
  }
  // runGui("TechnoGecko LED control", player, fullscreen);
}
