#include "glrenderer.h"

#include <GLFW/glfw3.h>
#include <math.h>
namespace jazzlights {

static constexpr double TWO_PI = 2 * 3.1415926;

static void renderLed(double cx, double cy, double r, jazzlights::RgbaColor color, int segments = 6) {
  glColor4f(color.red / 255.f, color.green / 255.f, color.blue / 255.f, color.alpha / 255.f);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(cx, cy);
  for (int i = 0; i <= segments; i++) {
    glVertex2f(cx + (r * cos(i * TWO_PI / segments)), cy + (r * sin(i * TWO_PI / segments)));
  }
  glEnd();
}

GLRenderer::GLRenderer(const Layout& l, Meters r) : layout_(l), ledRadius_(r) {}

void GLRenderer::render(InputStream<Color>& colors) {
  auto pti = begin(layout_);
  auto cli = colors.begin();
  while (cli != colors.end() && pti != end(layout_)) {
    Point pos = (*pti).coord;
    Color color = *cli;
    renderLed(pos.x, pos.y, ledRadius_, color.asRgba());
    ++pti;
    ++cli;
  }
}

}  // namespace jazzlights
