/**
 * Copyright 2018 Unisparks Project Developers
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * This file uses code from FastLED library (https://github.com/FastLED/FastLED)
 * licensed under the following terms:
 *
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2013 FastLED
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *  the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef UNISPARKS_MATH_H
#define UNISPARKS_MATH_H
#define _USE_MATH_DEFINES
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

namespace unisparks {

template<class T>
constexpr const T& min(const T& a, const T& b) {
  return (b < a) ? b : a;
}
template<class T>
constexpr const T& max(const T& a, const T& b) {
  return (a < b) ? b : a;
}
template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  return (v < lo) ? lo : (hi < v) ? hi : v;
}

template<class T>
constexpr T square(T x) { 
  return x * x; 
}

/**
 * Returns positive reminder
 */
inline double amod(double a, double b) {
  return fabs(fmod(a, b));
}

/**
 * Returns triangle wave with the period of 1 and amplitude of 1
 */
inline double triwave(double x) {
  // 1 - 2*abs[mod[2*x+0.5, 2]- 1]
  return 1 - 2 * fabs(amod(2 * x + 0.5, 2) - 1);
}

namespace internal {

static const uint8_t b_m16_interleave[] = {0, 49, 49, 41, 90, 27, 117, 10};


/// take abs() of a signed 8-bit uint8_t
inline int absi(int i) { return i < 0 ? -i : i; }

/// saturating 8x8 bit multiplication, with 8 bit result
/// @returns the product of i * j, capping at 0xFF
inline uint8_t qmul8(uint8_t i, uint8_t j) {
  int p = ((int)i * (int)(j));
  if (p > 255) {
    p = 255;
  }
  return p;
}

///  scale one byte by a second one, which is treated as
///  the numerator of a fraction whose denominator is 256
///  In other words, it computes i * (scale / 256)
///  4 clocks AVR with MUL, 2 clocks ARM
inline uint8_t scale8(uint8_t i, uint8_t scale) {
  return ((uint16_t)i * (uint16_t)(scale)) >> 8;
}

///  The "video" version of scale8 guarantees that the output will
///  be only be zero if one or both of the inputs are zero.  If both
///  inputs are non-zero, the output is guaranteed to be non-zero.
///  This makes for better 'video'/LED dimming, at the cost of
///  several additional cycles.
inline uint8_t scale8_video(uint8_t i, uint8_t scale) {
  uint8_t j = (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);
  return j;
}

/// Fast 8-bit approximation of sin(x). This approximation never varies more
/// than
/// 2% from the floating point value you'd get by doing
///
///     float s = (sin(x) * 128.0) + 128;
///
/// @param theta input angle from 0-255
/// @returns sin of theta, value between 0 and 255
inline uint8_t sin8(uint8_t theta) {
  uint8_t offset = theta;
  if (theta & 0x40) {
    offset = (uint8_t)255 - offset;
  }
  offset &= 0x3F; // 0..63

  uint8_t secoffset = offset & 0x0F; // 0..15
  if (theta & 0x40) {
    secoffset++;
  }

  uint8_t section = offset >> 4; // 0..3
  uint8_t s2 = section * 2;
  const uint8_t* p = b_m16_interleave;
  p += s2;
  uint8_t b = *p;
  p++;
  uint8_t m16 = *p;

  uint8_t mx = (m16 * secoffset) >> 4;

  int8_t y = mx + b;
  if (theta & 0x80) {
    y = -y;
  }

  y += 128;

  return y;
}

/// Fast 8-bit approximation of cos(x). This approximation never varies more
/// than
/// 2% from the floating point value you'd get by doing
///
///     float s = (cos(x) * 128.0) + 128;
///
/// @param theta input angle from 0-255
/// @returns sin of theta, value between 0 and 255
inline uint8_t cos8(uint8_t theta) { return sin8(theta + 64); }

inline uint8_t triwave8(uint8_t in) {
  if (in & 0x80) {
    in = 255 - in;
  }
  uint8_t out = in << 1;
  return out;
}

/// ease8InOutQuad: 8-bit quadratic ease-in / ease-out function
///                Takes around 13 cycles on AVR
inline uint8_t ease8InOutQuad(uint8_t i) {
  uint8_t j = i;
  if (j & 0x80) {
    j = 255 - j;
  }
  uint8_t jj = scale8(j, (j + 1));
  uint8_t jj2 = jj << 1;
  if (i & 0x80) {
    jj2 = 255 - jj2;
  }
  return jj2;
}

/// quadwave8: quadratic waveform generator.  Spends just a little more
///            time at the limits than 'sine' does.
inline uint8_t quadwave8(uint8_t in) { return ease8InOutQuad(triwave8(in)); }

/// add one byte to another, saturating at 0xFF
/// @param i - first byte to add
/// @param j - second byte to add
/// @returns the sum of i & j, capped at 0xFF
inline uint8_t qadd8(uint8_t i, uint8_t j) {
  unsigned int t = i + j;
  if (t > 255) {
    t = 255;
  }
  return t;
}

/// Add one byte to another, saturating at 0x7F
/// @param i - first byte to add
/// @param j - second byte to add
/// @returns the sum of i & j, capped at 0xFF
inline int8_t qadd7(int8_t i, int8_t j) {
  int16_t t = i + j;
  if (t > 127) {
    t = 127;
  }
  return t;
}

/// subtract one byte from another, saturating at 0x00
/// @returns i - j with a floor of 0
inline uint8_t qsub8(uint8_t i, uint8_t j) {
  int t = i - j;
  if (t < 0) {
    t = 0;
  }
  return t;
}

extern uint8_t sqrt16(uint16_t x);

/**
 * Online calculation of median, variance, and standard deviation
 */
class Welford {
 public:
  void reset() {
    n_ = 0;
    mean_ = m2_ = 0;
  }
  void add(double x) {
    n_++;
    double delta = x - mean_;
    mean_ += delta / n_;
    m2_ += delta * (x - mean_);
  }

  double mean() const { return mean_; }
  double variance() const { return n_ < 2 ? NAN : m2_ / (n_ - 1); }
  double stddev() const { return sqrt(variance()); }

 private:
  int n_;
  double mean_;
  double m2_;
};

constexpr inline int8_t avg7(int8_t i, int8_t j) {
  return ((i + j) >> 1) + (i & 0x1);
}

constexpr inline int16_t avg15(int16_t i, int16_t j) {
  return ((int32_t)((int32_t)(i) + (int32_t)(j)) >> 1) + (i & 0x1);
}

constexpr inline uint16_t scale16(uint16_t i, uint16_t scale) {
  return ((uint32_t)(i) * (1+(uint32_t)(scale))) / 65536;
}

inline int16_t lerp15by16(int16_t a, int16_t b, uint16_t frac) {
  int16_t result;
  if(b > a) {
    uint16_t delta = b - a;
    uint16_t scaled = scale16( delta, frac);
    result = a + scaled;
  } else {
    uint16_t delta = a - b;
    uint16_t scaled = scale16( delta, frac);
    result = a - scaled;
  }
  return result;
}

inline uint16_t beat88(uint16_t beats_per_minute_88, uint32_t elapsedTime) {
  // BPM is 'beats per minute', or 'beats per 60000ms'.
  // To avoid using the (slower) division operator, we
  // want to convert 'beats per 60000ms' to 'beats per 65536ms',
  // and then use a simple, fast bit-shift to divide by 65536.
  //
  // The ratio 65536:60000 is 279.620266667:256; we'll call it 280:256.
  // The conversion is accurate to about 0.05%, more or less,
  // e.g. if you ask for "120 BPM", you'll get about "119.93".
  return (elapsedTime * beats_per_minute_88 * 280) >> 16;
}

inline uint16_t beat16(uint16_t beats_per_minute, uint32_t elapsedTime) {
  // Convert simple 8-bit BPM's to full Q8.8 accum88's if needed
  if( beats_per_minute < 256) beats_per_minute <<= 8;
  return beat88(beats_per_minute, elapsedTime);
}

inline uint8_t beat8(uint16_t beats_per_minute, uint32_t elapsedTime) {
  // beat8 generates an 8-bit 'sawtooth' wave at a given BPM
  return beat16( beats_per_minute, elapsedTime) >> 8;
}

inline uint8_t beatsin8(uint16_t beats_per_minute, uint32_t elapsedTime,
                        uint8_t lowest = 0, uint8_t highest = 255,
                        uint8_t phase_offset = 0) {
  uint8_t beat = beat8(beats_per_minute, elapsedTime);
  uint8_t beatsin = sin8(beat + phase_offset);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

} // namespace private
} // namespace unisparks
#endif /* UNISPARKS_PROTO_H */
