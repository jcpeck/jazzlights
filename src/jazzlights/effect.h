#ifndef JAZZLIGHTS_EFFECT_H
#define JAZZLIGHTS_EFFECT_H
#include <string>

#include "jazzlights/frame.h"
#include "jazzlights/util/color.h"
#include "jazzlights/util/geom.h"
#include "jazzlights/util/log.h"

namespace jazzlights {

class Effect {
 public:
  virtual ~Effect() = default;

  virtual size_t contextSize(const Frame& frame) const = 0;
  virtual Color color(const Frame& frame, const Pixel& px) const = 0;
  virtual void begin(const Frame& frame) const = 0;
  virtual void rewind(const Frame& frame) const = 0;
  virtual std::string effectName(PatternBits pattern) const = 0;
};

template <typename STATE, typename PER_PIXEL_TYPE>
class XYIndexStateEffect : public Effect {
 public:
  virtual void innerBegin(const Frame& frame, STATE* state) const = 0;
  virtual void innerRewind(const Frame& frame, STATE* state) const = 0;
  virtual Color innerColor(const Frame& frame, STATE* state, const Pixel& px) const = 0;

  size_t contextSize(const Frame& frame) const override {
    return sizeof(STATE) + sizeof(PER_PIXEL_TYPE) * width(frame) * height(frame);
  }
  Color color(const Frame& frame, const Pixel& px) const override {
    XYIndex xyIndex = frame.xyIndexStore->FromPixel(px);
    x_ = xyIndex.xIndex;
    y_ = xyIndex.yIndex;
    return innerColor(frame, state(frame), px);
  }
  void begin(const Frame& frame) const override {
    saveFrame(frame);
    innerBegin(frame, state(frame));
  }
  void rewind(const Frame& frame) const override {
    saveFrame(frame);
    innerRewind(frame, state(frame));
  }

 protected:
  size_t x() const { return x_; }
  size_t y() const { return y_; }
  size_t w() const { return w_; }
  size_t h() const { return h_; }
  PER_PIXEL_TYPE& ps(size_t x, size_t y) const { return pixelState_[y * w() + x]; }
  PER_PIXEL_TYPE& ps() const { return ps(x(), y()); }

 private:
  void saveFrame(const Frame& frame) const {
    w_ = width(frame);
    h_ = height(frame);
    pixelState_ = pixelState(frame);
  }
  size_t width(const Frame& frame) const { return frame.xyIndexStore->xValuesCount(); }
  size_t height(const Frame& frame) const { return frame.xyIndexStore->yValuesCount(); }
  STATE* state(const Frame& frame) const { return static_cast<STATE*>(frame.context); }
  PER_PIXEL_TYPE* pixelState(const Frame& frame) const {
    return reinterpret_cast<PER_PIXEL_TYPE*>(reinterpret_cast<uint8_t*>(frame.context) + sizeof(STATE));
  }
  mutable size_t x_;
  mutable size_t y_;
  mutable size_t w_;
  mutable size_t h_;
  mutable PER_PIXEL_TYPE* pixelState_;
};

struct EmptyState {};

template <typename PER_PIXEL_TYPE>
class XYIndexEffect : public XYIndexStateEffect<EmptyState, PER_PIXEL_TYPE> {
 public:
  virtual void innerBegin(const Frame& frame) const = 0;
  virtual void innerRewind(const Frame& frame) const = 0;
  virtual Color innerColor(const Frame& frame, const Pixel& px) const = 0;
  void innerBegin(const Frame& frame, EmptyState* /*state*/) const override { innerBegin(frame); }
  void innerRewind(const Frame& frame, EmptyState* /*state*/) const override { innerRewind(frame); }
  Color innerColor(const Frame& frame, EmptyState* /*state*/, const Pixel& px) const override {
    return innerColor(frame, px);
  }
};

}  // namespace jazzlights
#endif  // JAZZLIGHTS_EFFECT_H
