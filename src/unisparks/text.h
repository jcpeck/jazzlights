#ifndef UNISPARKS_TEXT_H
#define UNISPARKS_TEXT_H

#include <Unisparks.h>

#if WEARABLE

namespace unisparks {

#define MATRIX_SIZE 25

bool displayText(const std::string& text,
                 CRGB pixels[MATRIX_SIZE],
                 CRGB textColor,
                 CRGB backgroundColor,
                 Milliseconds offsetMillis);

} // namespace unisparks

#endif // WEARABLE

#endif // UNISPARKS_TEXT_H
