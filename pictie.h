#include <math.h>
#include <stdint.h>

#include <memory>
#include <vector>

class Vector
{
public:
  double x;
  double y;

  Vector(double x, double y) : x(x), y(y) {}
  
  static Vector add(const Vector& a, const Vector& b) {
    return Vector(a.x + b.x, a.y + b.y);
  }

  static Vector sub(const Vector& a, const Vector& b) {
    return Vector(a.x - b.x, a.y - b.y);
  }

  static Vector scale(const Vector& a, double s) {
    return Vector(a.x * s, a.y * s);
  }
  
  Vector add(const Vector& b) const { return add(*this, b); }
  Vector sub(const Vector& b) const { return sub(*this, b); }
  Vector scale(double s) const { return scale(*this, s); }
  double magnitude() const { return sqrt(x * x + y * y); }
  Vector normalize(double length=1.0) const {
    return scale(length / magnitude());
  }
  Vector rotate90DegreesClockwise() const { return Vector(y, -x); }

  static Vector zero() { return Vector(0, 0); }
};

struct Segment
{
  Vector start;
  Vector end;

  Segment(const Vector& start, const Vector& end)
    : start(start), end(end) {}
};

struct Frame
{
  const Vector origin;
  const Vector edge1;
  const Vector edge2;

  Frame(const Vector& origin, const Vector& edge1, const Vector& edge2)
    : origin(origin), edge1(edge1), edge2(edge2) {}

  Vector project(const Vector& p) const {
    return origin.add(edge1.scale(p.x).add(edge2.scale(p.y)));
  }

  Segment project(const Segment& s) const {
    return Segment(project(s.start), project(s.end));
  }

  Frame project(const Frame& f) const {
    Vector newOrigin = project(f.origin);
    return Frame(newOrigin,
                 project(f.origin.add(f.edge1)).sub(newOrigin),
                 project(f.origin.add(f.edge2)).sub(newOrigin));
  }
};

struct Color
{
  uint8_t r;
  uint8_t g;
  uint8_t b;

  Color() : r(0), g(0), b(0) {}
  explicit Color(uint8_t gray) : r(gray), g(gray), b(gray) {}
  Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

  static Color black() { return Color(0); }
  static Color gray() { return Color(150); }
  static Color white() { return Color(255); }
};

enum class LineCapStyle { Butt, Square };
enum class LineWidthScaling { Scaled, Unscaled };

class DrawingContext
{
private:
  uint32_t width_;
  uint32_t height_;
  const Frame canvasFrame_;
  std::vector<Color> pixels_;

public:
  DrawingContext(uint32_t resolution);
  
  uint32_t width() const { return width_; };
  uint32_t height() const { return height_; };

  void fill(const Color& color);

  void drawTriangle(const Vector& a, const Vector &b, const Vector &c,
                    const Color& color);
  void drawQuad(const Vector& a, const Vector &b, const Vector &c,
                const Vector& d, const Color& color);
  void drawLine(const Vector& a, const Vector &b, const Color& color,
                double width, LineCapStyle lineCapStyle);
  void drawPixels(uint32_t pixelWidth, uint32_t pixelHeight, const Color* pixels,
                  const Vector& origin, const Vector& edge1, const Vector& edge2);

  bool writePPM(const char *fname);

  std::vector<Color> getPixels() const;
};
  
class Painter
{
protected:
  Painter() {}
public:
  virtual ~Painter() {}
  virtual void paint(DrawingContext &cx, const Frame& frame) const = 0;
};

using PainterPtr = std::shared_ptr<const Painter>;

PainterPtr triangle(const Vector& a, const Vector& b, const Vector& c,
                    const Color& color);
PainterPtr segments(const std::vector<Segment> segments, const Color& color,
                    double width = 1.0,
                    LineCapStyle lineCapStyle = LineCapStyle::Butt,
                    LineWidthScaling widthScaling = LineWidthScaling::Unscaled);
PainterPtr image(uint32_t width, uint32_t height, std::vector<Color>&& pixels);

PainterPtr transform(PainterPtr painter, const Vector& origin,
                     const Vector& corner1, const Vector& corner2);
PainterPtr over(PainterPtr a, PainterPtr b);

PainterPtr parallelogram(const Vector& origin, const Vector& edge1,
                         const Vector& edge2, const Color& color);
PainterPtr color(const Color& color);
PainterPtr flipHoriz(PainterPtr painter);
PainterPtr flipVert(PainterPtr painter);
PainterPtr rotate90(PainterPtr painter);
PainterPtr rotate180(PainterPtr painter);
PainterPtr rotate270(PainterPtr painter);
PainterPtr beside(PainterPtr a, PainterPtr b);
PainterPtr below(PainterPtr a, PainterPtr b);
PainterPtr beside3(PainterPtr a, PainterPtr b, PainterPtr c);
PainterPtr above3(PainterPtr a, PainterPtr b, PainterPtr c);
PainterPtr black();
PainterPtr gray();
PainterPtr white();

void paint(DrawingContext& cx, PainterPtr p);
