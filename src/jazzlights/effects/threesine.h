#ifndef JAZZLIGHTS_EFFECTS_THREESINE_H
#define JAZZLIGHTS_EFFECTS_THREESINE_H
#include "jazzlights/effects/functional.h"
#include "jazzlights/util/math.h"

namespace jazzlights {

auto threesine = []() {
  return effect("threesine", [=](const Frame& frame) {
    Coord w = width(frame);
    Coord h = height(frame);

    uint8_t sineOffset = 256 * frame.time / 8000;

    return [=](const Pixel& px) -> Color {
      // Calculate "sine" waves with varying periods
      // sin8 is used for speed; cos8, quadwave8, or triwave8 would also work
      // here
      double xx = (w > 64 ? 8.0 : 4.0) * px.coord.x / w;
      uint8_t sinDistanceR = qmul8(abs(px.coord.y * (255 / h) - sin8(sineOffset * 9 + xx * 16)), 2);
      uint8_t sinDistanceG = qmul8(abs(px.coord.y * (255 / h) - sin8(sineOffset * 10 + xx * 16)), 2);
      uint8_t sinDistanceB = qmul8(abs(px.coord.y * (255 / h) - sin8(sineOffset * 11 + xx * 16)), 2);

      return RgbaColor(255 - sinDistanceR, 255 - sinDistanceG, 255 - sinDistanceB);
    };
  });
};

}  // namespace jazzlights
#endif  // JAZZLIGHTS_EFFECTS_THREESINE_H
