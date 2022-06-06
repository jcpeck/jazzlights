#include "unisparks/player.hpp"

#include <cstdio>
#include <limits>
#include <stdlib.h>
#include <assert.h>
#include "unisparks/effects/chess.hpp"
#include "unisparks/effects/flame.hpp"
#include "unisparks/effects/glitter.hpp"
#include "unisparks/effects/glow.hpp"
#include "unisparks/effects/plasma.hpp"
#include "unisparks/effects/rainbow.hpp"
#include "unisparks/effects/rider.hpp"
#include "unisparks/effects/sequence.hpp"
#include "unisparks/effects/slantbars.hpp"
#include "unisparks/effects/solid.hpp"
#include "unisparks/effects/transform.hpp"
#include "unisparks/effects/threesine.hpp"
#include "unisparks/registry.hpp"
#include "unisparks/renderers/simple.hpp"
#include "unisparks/util/containers.hpp"
#include "unisparks/util/log.hpp"
#include "unisparks/util/math.hpp"
#include "unisparks/util/memory.hpp"
#include "unisparks/util/stream.hpp"
#include "unisparks/util/time.hpp"
#include "unisparks/version.hpp"

namespace unisparks {

using namespace internal;

int comparePrecedence(Precedence leftPrecedence,
                      const NetworkDeviceId& leftDeviceId,
                      Precedence rightPrecedence,
                      const NetworkDeviceId& rightDeviceId) {
  if (leftPrecedence < rightPrecedence) {
    return -1;
  } else if (leftPrecedence > rightPrecedence) {
    return 1;
  }
  return leftDeviceId.compare(rightDeviceId);
}

#if WEARABLE

auto calibration_effect = effect("calibration", [](const Frame& frame) {
  const bool blink = ((frame.time % 1000) < 500);
  return [ = ](const Pixel& pt) -> Color {
    const int32_t green = 0x00ff00, blue = 0x0000ff, red = 0xff0000;
    const int32_t yellow = green | red, purple = red | blue, white = 0xffffff;
    const int32_t orange = 0xffcc00;
    int32_t yColors[19] = {red, green, blue, yellow,
      purple, orange, white, blue, yellow,
      red, purple, green, orange,
      white, yellow, purple, green, blue, red};

    int32_t col = 0;
    const int32_t x = pt.coord.x;
    const int32_t y = pt.coord.y;
    if (y >= 0 && y < static_cast<int32_t>(sizeof(yColors) / sizeof(yColors[0]))) {
      col = yColors[y];
    }

    if (blink) {
      if (y == 0 &&
          (x == 0 || x == 6 || x == 14)) {
        col = 0;
      } else if (y == 1 &&
                (x == 5 || x == 13 || x == 19)) {
        col = 0;
      }
    }
    return Color(col);
  };
});

#endif // WEARABLE

auto black_effect = solid(BLACK, "black");
auto red_effect = solid(RED, "red");
auto green_effect = solid(GREEN, "green");
auto blue_effect = solid(BLUE, "blue");
auto purple_effect = solid(PURPLE, "purple");
auto cyan_effect = solid(CYAN, "cyan");
auto yellow_effect = solid(YELLOW, "yellow");
auto white_effect = solid(WHITE, "white");

auto red_glow_effect = glow(RED, "glow-red");
auto green_glow_effect = glow(GREEN, "glow-green");
auto blue_glow_effect = glow(BLUE, "glow-blue");
auto purple_glow_effect = solid(PURPLE, "glow-purple");
auto cyan_glow_effect = solid(CYAN, "glow-cyan");
auto yellow_glow_effect = solid(YELLOW, "glow-yellow");
auto white_glow_effect = solid(WHITE, "glow-white");

auto network_effect = [](NetworkStatus network_status, const std::string& name) {
  return effect(std::string("network-") + name, [ = ](const Frame& frame) {
    int32_t color = 0;
    switch (network_status) {
      case INITIALIZING:
        color = 0xff6600;
        break;
      case DISCONNECTED:
        color = 0xff6600;
        break;
      case CONNECTING:
        color = ((frame.time % 500) < 250) ? 0 : 0xff6600;
        break;
      case CONNECTED:
        color = 0x00ff00;
        break;
      case DISCONNECTING:
        color = 0xff6600;
        break;
      case CONNECTION_FAILED:
        color = 0xff0000;
        break;
    }
    return [ = ](const Pixel& pt) -> Color {
      const int32_t x = pt.coord.x;
      const int32_t y = pt.coord.y;
      if ((x % 5) == 1 || (y % 5) == 1) {
        return Color(0xff00ff); // purple
      }
      return Color(color);
    };
  });
};

auto network_effect_initializing = network_effect(INITIALIZING, "init");
auto network_effect_disconnected = network_effect(DISCONNECTED, "disconnected");
auto network_effect_connecting = network_effect(CONNECTING, "connecting");
auto network_effect_connected = network_effect(CONNECTED, "connected");
auto network_effect_disconnecting = network_effect(DISCONNECTING, "disconnecting");
auto network_effect_connection_failed = network_effect(CONNECTION_FAILED, "failed");

Effect* get_network_effect(NetworkStatus networkStatus) {
  switch (networkStatus) {
    case INITIALIZING: return &network_effect_initializing;
    case DISCONNECTED: return &network_effect_disconnected;
    case CONNECTING: return &network_effect_connecting;
    case CONNECTED: return &network_effect_connected;
    case DISCONNECTING: return &network_effect_disconnecting;
    case CONNECTION_FAILED: return &network_effect_connection_failed;
  }
  return &network_effect_connection_failed;
}

auto synctest = effect("synctest", [](const Frame& frame) {
    return [ = ](const Pixel& /*pt*/) -> Color {
      Color colors[] = {0xff0000, 0x00ff00, 0x0000ff, 0xffffff};
      return colors[int(frame.time / 1000) % 4];
    };
  });

constexpr bool patternIsReserved(PatternBits pattern) {
  // Patterns with lowest byte zero are reserved.
  return (pattern & 0xFF) == 0;
}

// bitNum is [1-32] starting from the highest bit.
constexpr bool patternbit(PatternBits pattern, uint8_t bitNum) {
  return (pattern & (1 << (sizeof(PatternBits) * 8 - bitNum))) != 0;
}

PatternBits computeNextPattern(PatternBits pattern) {
  static_assert(sizeof(PatternBits) == 4, "32bits");
  // This code is inspired by xorshift, amended to only require 32 bits of
  // state. This algorithm was informed by 10 minutes of Googling and a half
  // bottle of Malbec. It is guaranteed to produce numbers.
	pattern ^= pattern << 13;
	pattern ^= pattern >> 17;
	pattern ^= pattern << 5;
  pattern += 0x1337;
  // Apparently xorshift doesn't have great entropy in the lower bits, so let's
  // move those around just because we can.
  const uint8_t shift_offset = (pattern / 16384) % 32;
	pattern = (pattern << shift_offset) | (pattern >> (32 - shift_offset));
  if (pattern == 0) { pattern = 0xDF123456; }
  while (patternIsReserved(pattern)) {
    // Skip reserved patterns.
    pattern = computeNextPattern(pattern);
  }
  return pattern;
}

auto spin_rainbow_pattern = clone(rainbow());
auto spin_forest_pattern = clone(SpinPlasma(OCPforest));
auto spin_party_pattern = clone(SpinPlasma(OCPparty));
auto spin_cloud_pattern = clone(SpinPlasma(OCPcloud));
auto spin_ocean_pattern = clone(SpinPlasma(OCPocean));
auto spin_lava_pattern = clone(SpinPlasma(OCPlava));
auto spin_heat_pattern = clone(SpinPlasma(OCPheat));
auto flame_pattern = clone(flame());
auto glitter_pattern = clone(glitter());
auto threesine_pattern = clone(threesine());
auto rainbow_pattern = clone(rainbow());

Effect* patternFromBits(PatternBits pattern) {
  if (patternIsReserved(pattern)) {
    const uint8_t byte1 = (pattern >> 24) & 0xFF;
    const uint8_t byte2 = (pattern >> 16) & 0xFF;
    const uint8_t byte3 = (pattern >>  8) & 0xFF;
    if (byte1 == 0 && byte2 == 0) {
      switch (byte3) {
        case 0: return &black_effect;
        case 1: return &red_effect;
        case 2: return &green_effect;
        case 3: return &blue_effect;
        case 4: return &purple_effect;
        case 5: return &cyan_effect;
        case 6: return &yellow_effect;
        case 7: return &white_effect;
        case 8: return &red_glow_effect;
        case 9: return &green_glow_effect;
        case 10: return &blue_glow_effect;
        case 11: return &purple_glow_effect;
        case 12: return &cyan_glow_effect;
        case 13: return &yellow_glow_effect;
        case 14: return &white_glow_effect;
        case 15: return &synctest;
#if WEARABLE
        case 16: return &calibration_effect;
#endif  // WEARABLE
      }
    }
    return &red_effect;
  } else {
    if (patternbit(pattern, 1)) { // spin
      if (patternbit(pattern, 2)) { // nature
        if (patternbit(pattern, 3)) { // rainbow
          return &rainbow_pattern;
        } else { // frolick
          if (patternbit(pattern, 4)) { // forest
            return &spin_forest_pattern;
          } else { // party
            return &spin_party_pattern;
          }
        }
      } else { // hot&cold
        if (patternbit(pattern, 3)) { // cold
          if (patternbit(pattern, 4)) { // cloud
            return &spin_cloud_pattern;
          } else { // ocean
            return &spin_ocean_pattern;
          }
        } else { // hot
          if (patternbit(pattern, 4)) { // lava
            return &spin_lava_pattern;
          } else { // heat
            return &spin_heat_pattern;
          }
        }
      }
    } else { // custom
      if (patternbit(pattern, 2)) { // sparkly
        if (patternbit(pattern, 3)) { // flame
#if WEARABLE
          return &flame_pattern;
#else  // WEARABLE
          // TODO figure out why flame does not work on vehicles.
          return &spin_lava_pattern;
#endif  // WEARABLE
        } else { // glitter
          return &glitter_pattern;
        }
      } else { // shiny
        if (patternbit(pattern, 3)) { // threesine
          return &threesine_pattern;
        } else { // rainbow
          return &rainbow_pattern;
        }
      }
    }
  }
  fatal("Failed to pick an effect %s",
        displayBitsAsBinary(pattern).c_str());
}

Effect* Player::currentEffect() const {
  return patternFromBits(currentPattern_);
}

void render(const Layout& layout, Renderer* renderer,
            const Effect& effect, Frame effectFrame) {
  auto pixels = points(layout);
  auto colors = map(pixels, [&](Point pt) -> Color {
    Pixel px;
    px.coord = pt;
    px.frame = effectFrame;
    Color clr = effect.color(px);
    return clr;
  });

  if (renderer) {
    renderer->render(colors);
  }
}

Player::Player() {
  reset();
}

Player::~Player() {
  end();
}

void Player::end() {
  free(effectContext_);
  effectContext_ = nullptr;
  effectContextSize_ = 0;
}

void Player::reset() {
  end();

  ready_ = false;
  loop_ = false;
  strandCount_ = 0;
  currentPatternStartTime_ = 0;
  currentPattern_ = 0x12345678;
  nextPattern_ = computeNextPattern(currentPattern_);
  tempo_ = 120;
  metre_ = SIMPLE_QUADRUPLE;

  networks_.clear();
  powerLimited = false;

  lastLEDWriteTime_ = -1;
  lastUserInputTime_ = -1;

  fps_ = -1;
  lastFpsProbeTime_ = -1;
  framesSinceFpsProbe_ = -1;

  viewport_.origin.x = 0;
  viewport_.origin.y = 0;
  viewport_.size.height = 1;
  viewport_.size.width = 1;

  originatorEntries_.clear();
}

Player& Player::clearStrands() {
  strandCount_ = 0;
  return *this;
}

Player& Player::addStrand(const Layout& l, SimpleRenderFunc r) {
  return addStrand(l, make<SimpleRenderer>(r));
}

Player& Player::addStrand(const Layout& l, Renderer& r) {
  end();
  constexpr size_t MAX_STRANDS = sizeof(strands_) / sizeof(*strands_);
  if (strandCount_ >= MAX_STRANDS) {
    fatal("Trying to add too many strands, max=%d", MAX_STRANDS);
  }
  strands_[strandCount_++] = {&l, &r};
  return *this;
}

Player& Player::connect(Network* n) {
  end();
  info("Connecting network %s", n->name());
  networks_.push_back(n);
  ready_ = false;
  return *this;
}

void Player::begin() {
  for (Strand* s = strands_;
       s < strands_ + strandCount_; ++s) {
    viewport_ = merge(viewport_, unisparks::bounds(*s->layout));
  }

  int pxcnt = 0;
  for (Strand* s = strands_;
       s < strands_ + strandCount_; ++s) {
    pxcnt += s->layout->pixelCount();
  }
  info("Starting Unisparks player %s (v%s); strands: %d%s, pixels: %d, %s w %f h %f",
       BOOT_MESSAGE,
       UNISPARKS_VERSION,
       strandCount_,
       strandCount_ < 1 ? " (CONTROLLER ONLY!)" : "",
       pxcnt,
       !networks_.empty() ? "networked" : "standalone",
       viewport_.size.width * viewport_.size.height);

  ready_ = true;
  nextInner(timeMillis());
}

void Player::handleSpecial() {
  specialMode_++;
  if (specialMode_ > 6) {
    specialMode_ = 1;
  }
  info("Starting special mode %u", specialMode_);
}

void Player::stopSpecial() {
  if (specialMode_ == 0) {
    return;
  }
  info("Stopping special mode");
  specialMode_ = 0;
}

static constexpr Milliseconds kEffectDuration = 10 * ONE_SECOND;

void Player::render(NetworkStatus networkStatus, Milliseconds currentTime) {
  if (!ready_) {
    begin();
  }

  syncToNetwork(currentTime);

  if (currentTime - lastFpsProbeTime_ > ONE_SECOND) {
    fps_ = framesSinceFpsProbe_;
    lastFpsProbeTime_ = currentTime;
    framesSinceFpsProbe_ = 0;
  }
  framesSinceFpsProbe_++;

  if (currentTime - currentPatternStartTime_ > kEffectDuration && !loop_) {
    info("%u Exceeded effect duration, switching to next effect",
         currentTime);
    nextInner(currentTime);
  }

  static constexpr Milliseconds minLEDWriteTime = 10;
  if (lastLEDWriteTime_ >= 0 &&
      currentTime - minLEDWriteTime < lastLEDWriteTime_) {
    return;
  }
  lastLEDWriteTime_ = currentTime;

  const Effect* effect = currentEffect();

  switch (specialMode_) {
    case 1:
#if WEARABLE
      effect = &calibration_effect;
#endif // WEARABLE
      break;
    case 2:
      effect = get_network_effect(networkStatus);
      break;
    case 3:
      effect = &black_effect;
      break;
    case 4:
      effect = &red_effect;
      break;
    case 5:
      effect = &green_effect;
      break;
    case 6:
      effect = &blue_effect;
      break;
  }

  Frame efr = effectFrame(effect, currentTime);
  effect->rewind(efr);

  for (Strand* s = strands_;
       s < strands_ + strandCount_; ++s) {
    unisparks::render(*s->layout, s->renderer, *effect, efr);
  }
}

void Player::nextInner(Milliseconds currentTime) {
  updateToNewPattern(nextPattern_, computeNextPattern(nextPattern_),
                     /*newCurrentPatternStartTime=*/currentTime, currentTime);
}

void Player::next(Milliseconds currentTime) {
  lastUserInputTime_ = currentTime;
  nextInner(currentTime);
  for (Network* network : networks_) {
    network->triggerSendAsap(currentTime);
  }
}

Precedence getPrecedenceGain(Milliseconds epochTime,
                             Milliseconds currentTime,
                             Milliseconds duration,
                             Precedence maxGain) {
  if (epochTime < 0) {
    return 0;
  } else if (currentTime < epochTime) {
    return maxGain;
  } else if (currentTime - epochTime > duration) {
    return 0;
  }
  const Milliseconds timeDelta = currentTime - epochTime;
  if (timeDelta < duration / 10) {
    return maxGain;
  }
  return static_cast<uint64_t>(duration - timeDelta) * maxGain / duration;
}

Precedence addPrecedenceGain(Precedence startPrecedence,
                             Precedence gain) {
  if (startPrecedence >= std::numeric_limits<Precedence>::max() - gain) {
    return std::numeric_limits<Precedence>::max();
  }
  return startPrecedence + gain;
}

static constexpr Milliseconds kInputDuration = 10 * 60 * 1000;  // 10min.

Precedence Player::getLocalPrecedence(Milliseconds currentTime) {
  return addPrecedenceGain(basePrecedence_,
                           getPrecedenceGain(lastUserInputTime_, currentTime,
                                             kInputDuration, precedenceGain_));
}

NetworkDeviceId Player::getLocalDeviceId(Milliseconds /*currentTime*/) {
  for (Network* network : networks_) {
    NetworkDeviceId localDeviceId = network->getLocalDeviceId();
    if (localDeviceId != NetworkDeviceId()) {
      return localDeviceId;
    }
  }
  return NetworkDeviceId();
}

void Player::updateToNewPattern(PatternBits newCurrentPattern,
                                PatternBits newNextPattern,
                                Milliseconds newCurrentPatternStartTime,
                                Milliseconds currentTime) {
  currentPatternStartTime_ = newCurrentPatternStartTime;
  nextPattern_ = newNextPattern;
  if (newCurrentPattern != currentPattern_) {
    currentPattern_ = newCurrentPattern;
    Effect* effect = currentEffect();
    info("%u Switching to pattern %s %s",
        currentTime,
        effect->name().c_str(),
        displayBitsAsBinary(currentPattern_).c_str());
    effect->begin(effectFrame(effect, currentTime));
    lastLEDWriteTime_ = -1;
  }
  if (networks_.empty()) {
    info("%u not setting messageToSend without networks", currentTime);
    return;
  }
  NetworkMessage messageToSend;
  messageToSend.originator = getFollowedDeviceId(currentTime);
  messageToSend.sender = getLocalDeviceId(currentTime);
  if (messageToSend.sender == NetworkDeviceId()) {
    info("%u not setting messageToSend without localDeviceId", currentTime);
    return;
  }
  messageToSend.currentPattern = currentPattern_;
  messageToSend.nextPattern = nextPattern_;
  messageToSend.currentPatternStartTime = currentPatternStartTime_;
  messageToSend.precedence = getOutgoingPrecedence(currentTime);
  for (Network* network : networks_) {
    if (!network->shouldEcho() && followedNetwork_ == network) {
      debug("%u Not echoing for %s to %s ",
            currentTime, network->name(),
            networkMessageToString(messageToSend, currentTime).c_str());
      network->disableSending(currentTime);
      continue;
    }
    debug("%u Setting messageToSend for %s to %s ",
          currentTime, network->name(),
          networkMessageToString(messageToSend, currentTime).c_str());
    network->setMessageToSend(messageToSend, currentTime);
  }
}

Player::OriginatorEntry* Player::getOriginatorEntry(NetworkDeviceId originator,
                                                    Milliseconds /*currentTime*/) {
  OriginatorEntry* entry = nullptr;
  for (OriginatorEntry& e : originatorEntries_) {
    if (e.originator == originator) {
      return &e;
    }
  }
  return entry;
}

static constexpr Milliseconds kOriginationTimeOverride = 6000;
static constexpr Milliseconds kOriginationTimeDiscard = 9000;

static_assert(kOriginationTimeOverride < kOriginationTimeDiscard,
  "Inverting these can lead to retracting an originator "
  "while disallowing picking a replacement.");
static_assert(kOriginationTimeDiscard < kEffectDuration,
  "Inverting these can lead to keeping an originator "
  "past the end of its intended next pattern.");

NetworkDeviceId Player::pickLeader(Milliseconds currentTime) {
  // TODO remove outgoing vs incoming precendence and remove depreciation across hops.
  Precedence precedence = getLocalPrecedence(currentTime);
  NetworkDeviceId originator = getLocalDeviceId(currentTime);
  if (lastUserInputTime_ >= 0 &&
      lastUserInputTime_ < currentTime &&
      currentTime - lastUserInputTime_ < kInputDuration) {
    // Pick ourselves as leader when there's been recent user input.
    return originator;
  }
  for (const OriginatorEntry& e : originatorEntries_) {
    if (e.retracted) {
      continue;
    }
     if (currentTime > e.lastOriginationTime + kOriginationTimeDiscard) {
       // TODO figure out what we're sending when we're in the next pattern
       continue;
     }
     if (currentTime > e.currentPatternStartTime + 2 * kEffectDuration) {
       continue;
     }
     if (comparePrecedence(e.precedence, e.originator,
                           precedence, originator) <= 0) {
       continue;
     }
     precedence = e.precedence;
     originator = e.originator;
  }
  return originator;
}

void Player::handleReceivedMessage(NetworkMessage message, Milliseconds currentTime) {
  if (message.sender == getLocalDeviceId(currentTime)) {
    debug("%u Ignoring received message that we sent %s",
          currentTime, networkMessageToString(message, currentTime).c_str());
    return;
  }
  if (message.originator == getLocalDeviceId(currentTime)) {
    debug("%u Ignoring received message that we originated %s",
          currentTime, networkMessageToString(message, currentTime).c_str());
    return;
  }
  OriginatorEntry* entry = getOriginatorEntry(message.originator, currentTime);
  if (entry == nullptr) {
    originatorEntries_.push_back(OriginatorEntry());
    entry = &originatorEntries_.back();
    entry->originator = message.originator;
    entry->precedence = message.precedence;
    entry->currentPattern = message.currentPattern;
    entry->nextPattern = message.nextPattern;
    entry->currentPatternStartTime = message.currentPatternStartTime;
    entry->lastOriginationTime = message.lastOriginationTime;
    entry->nextHopDevice = message.sender;
    entry->nextHopNetwork = message.receiptNetwork;
    entry->numHops = message.numHops;
    entry->retracted = false;
  } else {
    // The concept behind this is that we build a tree rooted at each originator
    // using a variant of the Bellman-Ford algorithm. We then only ever listen
    // to our next hop on the way to the originator to avoid oscillating between
    // neighbors. To avoid loops in this tree, we ignore any update that has same
    // or more hops than our currently saved one. To allow us to recover from
    // situations where the originator has moved further away in the network, we
    // accept those updates if they're more recent by kOriginationTimeOverride
    // than what we've seen so far. This is based on the theoretical points made
    // in Section 2 of RFC 8966 - we can say that while much simpler and less
    // powerful, this is inspired by the Babel Routing Protocol.
    if (entry->nextHopDevice != message.sender ||
        entry->nextHopNetwork != message.receiptNetwork) {
      bool changeNextHop = false;
      if (message.numHops < entry->numHops) {
        info("%u Switching to better nextHop due to numHops",
             currentTime);
        changeNextHop = true;
      } else if (message.lastOriginationTime > entry->lastOriginationTime + kOriginationTimeOverride) {
        info("%u Switching to better nextHop due to originationTime",
             currentTime);
        changeNextHop = true;
      }
      if (changeNextHop) {
        entry->nextHopDevice = message.sender;
        entry->nextHopNetwork = message.receiptNetwork;
        entry->numHops = message.numHops;
      }
    }

    if (entry->nextHopDevice == message.sender &&
        entry->nextHopNetwork == message.receiptNetwork) {
      entry->precedence = message.precedence;
      entry->currentPattern = message.currentPattern;
      entry->nextPattern = message.nextPattern;
      entry->currentPatternStartTime = message.currentPatternStartTime;
      entry->lastOriginationTime = message.lastOriginationTime;
      entry->retracted = false;
    }
  }
  // If this sender is following another originator from what we previously heard,
  // retract any previous entries from them.
  for (OriginatorEntry& e : originatorEntries_) {
    if (e.nextHopDevice == message.sender &&
        e.nextHopNetwork == message.receiptNetwork &&
        e.originator != message.originator &&
        !e.retracted) {
      e.retracted = true;
      info("%u Retracting entry for originator",
             currentTime);
    }
  }

  Precedence incomingPrecedence = getIncomingPrecedence(currentTime);
  if (currentTime - message.currentPatternStartTime > kEffectDuration && !loop_) {
    // Ignore this message because the sender already switched effects.
    info("%u Ignoring received message %s past duration %u",
         currentTime, networkMessageToString(message, currentTime).c_str(), kEffectDuration);
    return;
  }
  lastLEDWriteTime_ = -1;
  updateToNewPattern(message.currentPattern, message.nextPattern,
                     message.currentPatternStartTime, currentTime);
}

void Player::syncToNetwork(Milliseconds currentTime) {
  // First listen on all networks.
  for (Network* network : networks_) {
    for (NetworkMessage receivedMessage :
        network->getReceivedMessages(currentTime)) {
      handleReceivedMessage(receivedMessage, currentTime);
    }
  }
  // Then give all networks the opportunity to send.
  for (Network* network : networks_) {
    network->runLoop(currentTime);
  }
}

void Player::loopOne() {
  if (loop_) {
    return;
  }
  info("Looping");
  loop_ = true;
}

Frame Player::effectFrame(const Effect* effect, Milliseconds currentTime) {
  // Ensure effectContext_ is big enough for this effect.
  const size_t effectContextSize = effect->contextSize({viewport_, nullptr});
  if (effectContextSize > effectContextSize_) {
    info("%u realloc context size from %zu to %zu (%s w %f h %f)",
         currentTime, effectContextSize_, effectContextSize,
         effect->name().c_str(), viewport_.size.width * viewport_.size.height);
    effectContextSize_ = effectContextSize;
    effectContext_ = realloc(effectContext_, effectContextSize_);
  }

  Frame frame;
  frame.animation.viewport = viewport_;
  frame.animation.context = effectContext_;
  frame.time = currentTime - currentPatternStartTime_;
  frame.tempo = tempo_;
  frame.metre = metre_;
  return frame;
}

const char* Player::command(const char* req) {
  static char res[256];
  const size_t MAX_CMD_LEN = 16;
  bool responded = false;

  if (!strncmp(req, "status?", MAX_CMD_LEN)) {
    // do nothing
  } else  if (!strncmp(req, "next", MAX_CMD_LEN)) {
    next(timeMillis());
  } else {
    snprintf(res, sizeof(res), "! unknown command");
    responded = true;
  }
  if (!responded) {
    snprintf(res, sizeof(res), "play %s %d",
             currentEffect()->name().c_str(), timeMillis() - currentPatternStartTime_);
  }
  debug("[%s] -> [%s]", req, res);
  return res;
}

} // namespace unisparks
