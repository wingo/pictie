#include <emscripten/bind.h>

#include "pictie.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(interface_tests) {
  class_<Vector>("Vector")
    .class_function("zero", &Vector::zero)
    .constructor<double, double>()
    .property("x", &Vector::x)
    .property("y", &Vector::y)
    .function("add", static_cast<Vector (Vector::*)(const Vector&) const>(&Vector::add))
    .function("sub", static_cast<Vector (Vector::*)(const Vector&) const>(&Vector::sub))
    .function("scale", static_cast<Vector (Vector::*)(double) const>(&Vector::scale))
    .function("magnitude", static_cast<double (Vector::*)() const>(&Vector::magnitude))
    .function("normalize", static_cast<Vector (Vector::*)(double) const>(&Vector::normalize))
    .function("rotate90DegreesClockwise",  static_cast<Vector (Vector::*)() const>(&Vector::rotate90DegreesClockwise))
    ;
  register_vector<Vector>("VectorArray");

  class_<Frame>("Frame")
    .constructor<Vector, Vector, Vector>()
    .property("origin", &Frame::origin)
    .property("edge1", &Frame::edge1)
    .property("edge2", &Frame::edge2)
    // FIXME: Add support for other overloads of "project".
    .function("project", static_cast<Vector (Frame::*)(const Vector&) const>(&Frame::project))
    ;

  class_<Color>("Color")
    .class_function("black", &Color::black)
    .class_function("gray", &Color::gray)
    .class_function("white", &Color::white)
    .class_function("blue", &Color::blue)
    .class_function("red", &Color::red)
    .constructor<>()
    .constructor<uint8_t>()
    .constructor<uint8_t, uint8_t, uint8_t>()
    .property("r", &Color::r)
    .property("g", &Color::g)
    .property("b", &Color::b)
    ;
  register_vector<Color>("ColorArray");

  enum_<LineCapStyle>("LineCapStyle")
    .value("Butt", LineCapStyle::Butt)
    .value("Square", LineCapStyle::Square)
    ;

  enum_<LineWidthScaling>("LineWidthScaling")
    .value("Scaled", LineWidthScaling::Scaled)
    .value("Unscaled", LineWidthScaling::Unscaled)
    ;

  class_<DrawingContext>("DrawingContext")
    .constructor<uint32_t>()
    .function("fill", &DrawingContext::fill)
    .function("drawTriangle", &DrawingContext::drawTriangle)
    .function("drawQuad", &DrawingContext::drawQuad)
    .function("drawLine", &DrawingContext::drawLine)
    // FIXME: .function("drawPixels", &DrawingContext::drawPixels)
    // FIXME: .function("writePPM", &DrawingContext::writePPM)
    .function("getPixels", &DrawingContext::getPixels)
    ;

  class_<Painter>("Painter")
    .function("paint", &Painter::paint)
    .smart_ptr<std::shared_ptr<const Painter>>("Painter")
    ;

  function("triangle", &triangle);
  function("path", &path);
  function("image", &image);
  function("transform", &transform);
  function("over", &over);
  function("parallelogram", &parallelogram);
  function("color", &color);
  function("flipHoriz", &flipHoriz);
  function("flipVert", &flipVert);
  function("rotate90", &rotate90);
  function("rotate180", &rotate180);
  function("rotate270", &rotate270);
  function("beside", &beside);
  function("below", &below);
  function("beside3", &beside3);
  function("above3", &above3);
  function("black", &black);
  function("gray", &gray);
  function("white", &white);
  function("blue", &blue);
  function("red", &red);

  function("paint", &paint);
}
