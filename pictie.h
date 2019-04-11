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
  
  static Vector add(const Vector& a, const Vector& b) 
  {
    return Vector(a.x + b.x, a.y + b.y);
  }

  static Vector sub(const Vector& a, const Vector& b) 
  {
    return Vector(a.x - b.x, a.y - b.y);
  }

  static Vector scale(const Vector& a, double s)
  {
    return Vector(a.x * s, a.y * s);
  }
  
  Vector add(const Vector& b) const { return add(*this, b); }
  Vector sub(const Vector& b) const { return sub(*this, b); }
  Vector scale(double s) const { return scale(*this, s); }
  double magnitude() const {
    return sqrt(x * x + y * y);
  }
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
    return Vector::add(origin,
                       Vector::add(edge1.scale(p.x),
                                   edge2.scale(p.y)));
  }

  Segment project(const Segment& s) const {
    return Segment(project(s.start), project(s.end));
  }

  Frame project(const Frame& f) const {
    Vector newOrigin = project(f.origin);
    return Frame(newOrigin,
                 Vector::sub(project(f.edge1), newOrigin),
                 Vector::sub(project(f.edge2), newOrigin));
  }
};

struct AffineTransform
{
  const double xx;
  const double xy;
  const double yx;
  const double yy;
  const double x0;
  const double y0;

  AffineTransform(double xx, double xy, double yx,
                  double yy, double x0, double y0)
    : xx(xx), xy(xy), yx(yx), yy(yy), x0(x0), y0(y0) {}

  explicit AffineTransform(const Frame& frame)
    : xx(frame.edge1.x), xy(frame.edge2.x),
      yx(frame.edge1.y), yy(frame.edge2.y),
      x0(frame.origin.x), y0(frame.origin.x) {}
  
  static AffineTransform compose(const AffineTransform& a,
                                 const AffineTransform& b) {
    return AffineTransform(a.xx * b.xx + a.xy * b.yx,
                           a.xx * b.xy + a.xy * b.yy,
                           a.yx * b.xx + a.yy * b.yx,
                           a.yx * b.xy + a.yy * b.yy,
                           a.xx * b.x0 + a.xy * b.y0 + a.x0,
                           a.yx * b.x0 + a.yy * b.y0 + a.y0);
  }

  AffineTransform compose(const AffineTransform& b) const {
    return compose(*this, b);
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

class DrawingContext
{
private:
  uint32_t width_;
  uint32_t height_;
  const Frame canvasFrame_;
  std::unique_ptr<Color[]> pixels_;

public:
  DrawingContext(uint32_t resolution)
    : width_(resolution), height_(resolution),
      canvasFrame_(Vector(0,resolution), Vector(resolution,0), Vector(0,-resolution)),
      pixels_(new Color[resolution * resolution])
  {
    fill(Color::white());
  }
  
  void fill(const Color& color) {
    for (uint32_t y = 0; y < height_; y++)
      for (uint32_t x = 0; x < width_; x++)
        pixels_[y * width_ + x] = color;
  }

  static inline bool rightOf(const Vector &a, const Vector &b, const Vector &c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) > 0;
  }

  static bool insideTriangle(const Vector& a, const Vector &b, const Vector &c,
                             const Vector& p) {
    return rightOf(a, b, p) && rightOf(b, c, p) && rightOf(c, a, p);
  }

  void drawTriangle(const Vector& a, const Vector &b, const Vector &c,
                    const Color& color) {
    const Vector ca = canvasFrame_.project(a);
    const Vector cb = canvasFrame_.project(b);
    const Vector cc = canvasFrame_.project(c);
    for (uint32_t y = 0; y < height_; y++)
      for (uint32_t x = 0; x < width_; x++)
        if (insideTriangle(ca, cb, cc, Vector(double(x) + 0.5, double(y) + 0.5)))
          pixels_[y * width_ + x] = color;
  }

  void drawRectangle(const Vector& a, const Vector &b, const Vector &c,
                     const Vector& d, const Color& color) {
    drawTriangle(a, b, c, color);
    drawTriangle(c, d, a, color);
  }
    
  void drawLine(const Vector& a, const Vector &b, const Color& color,
                double width, LineCapStyle lineCapStyle) {
    Vector q = b.sub(a).normalize(width * 0.5);
    Vector r = q.rotate90DegreesClockwise();
    
    switch (lineCapStyle) {
    case LineCapStyle::Butt:
      drawRectangle(a.add(r), a.sub(r), b.add(r), b.sub(r), color);
      break;
    case LineCapStyle::Square:
      drawRectangle(a.sub(q).add(r), a.sub(q).sub(r),
                    b.add(q).add(r), b.add(q).sub(r),
                    color);
      break;
    }
  }
    
  // Temporary implementation.
  void drawPixels(uint32_t pixelWidth, uint32_t pixelHeight, const Color* pixels,
                  const Vector& origin, const Vector& edge1, const Vector& edge2) {
    drawRectangle(origin, origin.add(edge1),
                  origin.add(edge1).add(edge2), origin.add(edge2),
                  Color::black());
  }

  void writePPM(const char *fname) {
    FILE *f = fopen(fname, "w");
    if (!f) {
      fprintf(stderr, "failed to create %s: ", fname);
      perror(NULL);
      return;
    }
    fprintf(f, "P6\n%u %u\n255\n", width_, height_);
    for (uint32_t y = 0; y < height_; y++) {
      for (uint32_t x = 0; x < width_; x++) {
        const Color& p = pixels_[y * width_ + x];
        fprintf(f, "%c%c%c", p.r, p.g, p.b);
      }
    }
    if (fflush(f) != 0)
      perror("writing PPM file failed");
    fclose(f);
  }
};
  
class Painter
{
protected:
  Painter() {};
public:
  virtual void paint(DrawingContext &cx, const Frame& frame) const = 0;
};

class ColorPainter : public Painter
{
private:
  const Color color_;

public:
  explicit ColorPainter(const Color& color) : color_(color) {}

  virtual void paint(DrawingContext &cx, const Frame& frame) const
  {
    cx.drawRectangle(frame.origin,
                     frame.origin.add(frame.edge1),
                     frame.origin.add(frame.edge1).add(frame.edge2),
                     frame.origin.add(frame.edge2),
                     color_);
  }
};
                      
class SegmentsPainter : public Painter
{
private:
  std::vector<Segment> segments_;
  const Color color_;
  double width_;
  LineCapStyle lineCapStyle_;

public:
  SegmentsPainter(std::vector<Segment> segments, const Color& color,
                  double width = 1.0,
                  LineCapStyle lineCapStyle = LineCapStyle::Butt)
    : segments_(segments), color_(color), width_(width),
      lineCapStyle_(lineCapStyle) {}

  virtual void paint(DrawingContext &cx, const Frame& frame) const
  {
    for (auto segment : segments_)
      cx.drawLine(frame.project(segment.start),
                  frame.project(segment.end),
                  color_, width_, lineCapStyle_);
  }
};
                      
class ImagePainter : public Painter
{
private:
  uint32_t width_;
  uint32_t height_;
  std::unique_ptr<const Color[]> pixels_;

public:
  // Takes ownership of pixels.
  ImagePainter(uint32_t width, uint32_t height,
               std::unique_ptr<const Color[]> pixels)
    : width_(width), height_(height), pixels_(std::move(pixels))
  {}
  
  static ImagePainter* fromPPM(const char *fname) {
    FILE *f = fopen(fname, "r");
    if (!f) {
      fprintf(stderr, "failed to open %s: ", fname);
      perror(NULL);
      return nullptr;
    }
    unsigned int width = 0;
    unsigned int height = 0;
    if (fscanf(f, "P6\n%u %u\n255\n", &width, &height) != 2) {
      fprintf(stderr, "expected PPM header on %s\n", fname);
      fclose(f);
      return nullptr;
    }
    if (width * height == 0) {
      fprintf(stderr, "bad dimensions for PPM file %s: %u x %u\n", fname,
              width, height);
      fclose(f);
      return nullptr;
    }
    
    std::unique_ptr<Color[]> pixels(new Color[width * height]);
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        int r, g, b;
        r = fgetc(f); g = fgetc(f); b = fgetc(f);
        if (r == EOF || g == EOF || b == EOF) {
          fprintf(stderr, "got EOF while reading PPM data from %s\n", fname);
          fclose(f);
          return nullptr;
        }
        pixels[y * width + x] = Color(r, g, b);
      }
    }

    if (fgetc(f) != EOF) {
      fprintf(stderr, "expected EOF after reading PPM data from %s\n", fname);
      fclose(f);
      return nullptr;
    }
      
    fclose(f);

    return new ImagePainter(width, height, std::move(pixels));
  }

  virtual void paint(DrawingContext &cx, const Frame& frame) const
  {
    cx.drawPixels(width_, height_, pixels_.get(), frame.origin, frame.edge1,
                  frame.edge2);
  }
};
                      
class TransformPainter : public Painter
{
private:
  std::shared_ptr<const Painter> painter_;
  const Frame frame_;

public:
  TransformPainter(std::shared_ptr<const Painter> painter,
                   const Vector& origin, const Vector& corner1,
                   const Vector& corner2)
    : painter_(painter),
      frame_(origin, corner1.sub(origin), corner2.sub(origin))
  {}
  
  virtual void paint(DrawingContext &cx, const Frame& frame) const
  {
    painter_->paint(cx, frame.project(frame_));
  }
};

class SuperposePainter : public Painter
{
private:
  std::shared_ptr<const Painter> first_;
  std::shared_ptr<const Painter> second_;

public:
  SuperposePainter(std::shared_ptr<const Painter> first,
                   std::shared_ptr<const Painter> second)
    : first_(first), second_(second) {}
  
  virtual void paint(DrawingContext &cx, const Frame& frame) const
  {
    first_->paint(cx, frame);
    second_->paint(cx, frame);
  }
};

std::shared_ptr<const Painter>
color(const Color& color) {
  return std::shared_ptr<const Painter>(new ColorPainter(color));
}

std::shared_ptr<const Painter>
segments(std::vector<Segment> segments, const Color& color,
         double width = 1.0, LineCapStyle lineCapStyle = LineCapStyle::Butt) {
  return std::shared_ptr<const Painter>(new SegmentsPainter(std::move(segments), color, width, lineCapStyle));
}

std::shared_ptr<const Painter>
image(uint32_t width, uint32_t height, std::unique_ptr<Color[]> pixels) {
  return std::shared_ptr<const Painter>(new ImagePainter(width, height, std::move(pixels)));
}

std::shared_ptr<const Painter>
transform(std::shared_ptr<const Painter> painter,
          const Vector& origin, const Vector& corner1,
          const Vector& corner2) {
  return std::shared_ptr<const Painter>(new TransformPainter(painter, origin, corner1, corner1));
}

std::shared_ptr<const Painter>
superpose(std::shared_ptr<const Painter> a, std::shared_ptr<const Painter> b) {
  return std::shared_ptr<const Painter>(new SuperposePainter(a, b));
}

std::shared_ptr<const Painter>
flipHoriz(std::shared_ptr<const Painter> painter) {
  return transform(painter, Vector(1, 0), Vector(0, 0), Vector(1, 1));
};

std::shared_ptr<const Painter>
flipVert(std::shared_ptr<const Painter> painter) {
  return transform(painter, Vector(1, 0), Vector(0, 0), Vector(1, 1));
};

std::shared_ptr<const Painter>
rotate90(std::shared_ptr<const Painter> painter) {
  return transform(painter,Vector(1, 0), Vector(1, 1), Vector(0, 0));
};

std::shared_ptr<const Painter>
rotate180(std::shared_ptr<const Painter> painter) {
  return rotate90(rotate90(painter));
};

std::shared_ptr<const Painter>
rotate270(std::shared_ptr<const Painter> painter) {
  return rotate90(rotate90(rotate90(painter)));
};

std::shared_ptr<const Painter>
beside(std::shared_ptr<const Painter> a, std::shared_ptr<const Painter> b) {
  return superpose(transform(a, Vector(0,0), Vector(0.5,0), Vector(0,1)),
                   transform(b, Vector(0.5,0), Vector(1,0), Vector(0.5,1)));
};

std::shared_ptr<const Painter>
below(std::shared_ptr<const Painter> a, std::shared_ptr<const Painter> b) {
  return superpose(transform(a, Vector(0,0.5), Vector(1,0.5), Vector(0,1)),
                   transform(b, Vector(0,0), Vector(1,0), Vector(0,0.5)));
};

std::shared_ptr<const Painter>
beside3(std::shared_ptr<const Painter> a, std::shared_ptr<const Painter> b,
        std::shared_ptr<const Painter> c) {
  return superpose
    (transform(a, Vector(0,2./3.), Vector(1,2./3.), Vector(0,1)),
     superpose(transform(b, Vector(0,1./3.), Vector(1,1./3.), Vector(0,2/3.)),
               transform(c, Vector(0,0), Vector(1,0), Vector(0,1./3.))));
};

std::shared_ptr<const Painter>
above3(std::shared_ptr<const Painter> a, std::shared_ptr<const Painter> b,
       std::shared_ptr<const Painter> c) {
  return superpose
    (transform(a, Vector(0,0), Vector(1./3.,0), Vector(0,1)),
     superpose(transform(b, Vector(1/3.,0), Vector(2/3.,0), Vector(1/3.,1)),
               transform(b, Vector(2/3.,0), Vector(1,0), Vector(2/3.,1))));
};

std::shared_ptr<const Painter> black() { return color(Color::black()); }
std::shared_ptr<const Painter> gray() { return color(Color::gray()); }
std::shared_ptr<const Painter> white() { return color(Color::white()); }

std::shared_ptr<const Painter>
vects(std::vector<Vector> vects) {
  std::vector<Segment> segs;
  for (size_t i = 0; i + 1 < vects.size(); i++)
    segs.push_back(Segment(vects[i], vects[i+1]));
  return segments(segs, Color::black(), 0.01, LineCapStyle::Butt);
}

std::shared_ptr<const Painter>
markOfZorro() {
  return vects({Vector(.1,.9), Vector(.8,.9), Vector(.1,.2), Vector(.9,.3)});
}

// (define diagonal-shading (procedure->painter (λ (x y) (* 100 (+ x y)))))
// (define einstein         (bitmap->painter einstein-file))

std::shared_ptr<const Painter>
grid(double width, double height, std::vector<Segment> segs) {
  Frame f(Vector(0,0), Vector(1/width,0), Vector(1/height,0));
  std::vector<Segment> scaled;
  for (auto s : segs)
    scaled.push_back(f.project(s));
  return segments(scaled, Color::black(), 0.01, LineCapStyle::Butt);
}

std::shared_ptr<const Painter>
escher() {
  auto p =
    grid(16, 16, 
         {{{ 4,  4}, { 6,  0}}, {{ 0,  3}, { 3,  4}}, {{ 3,  4}, { 0,  8}},
          {{ 0,  8}, { 0,  3}}, {{ 4,  5}, { 7,  6}}, {{ 7,  6}, { 4, 10}},
          {{ 4, 10}, { 4,  5}}, {{11,  0}, {10,  4}}, {{10,  4}, { 8,  8}},
          {{ 8,  8}, { 4, 13}}, {{ 4, 13}, { 0, 16}}, {{11,  0}, {14,  2}},
          {{14,  2}, {16,  2}}, {{10,  4}, {13,  5}}, {{13,  5}, {16,  4}},
          {{ 9,  6}, {12,  7}}, {{12,  7}, {16,  6}}, {{ 8,  8}, {12,  9}},
          {{12,  9}, {16,  8}}, {{ 8, 12}, {16, 10}}, {{ 0, 16}, { 6, 15}},
          {{ 6, 15}, { 8, 16}}, {{ 8, 16}, {12, 12}}, {{12, 12}, {16, 12}},
          {{10, 16}, {12, 14}}, {{12, 14}, {16, 13}}, {{12, 16}, {13, 15}},
          {{13, 15}, {16, 14}}, {{14, 16}, {16, 15}}, {{16,  0}, {16,  8}},
          {{16, 12}, {16, 16}}});
  auto q =
    grid(16, 16,
         {{{ 2,  0}, { 4,  5}}, {{ 4,  5}, { 4,  7}}, {{ 4,  0}, { 6,  5}},
          {{ 6,  5}, { 6,  7}}, {{ 6,  0}, { 8,  5}}, {{ 8,  5}, { 8,  8}},
          {{ 8,  0}, {10,  6}}, {{10,  6}, {10,  9}}, {{10,  0}, {14, 11}},
          {{12,  0}, {13,  4}}, {{13,  4}, {16,  8}}, {{16,  8}, {15, 10}},
          {{15, 10}, {16, 16}}, {{16, 16}, {12, 10}}, {{12, 10}, { 6,  7}},
          {{ 6,  7}, { 4,  7}}, {{ 4,  7}, { 0,  8}}, {{13,  0}, {16,  6}},
          {{14,  0}, {16,  4}}, {{15,  0}, {16,  2}}, {{ 0, 10}, { 7, 11}},
          {{ 9, 12}, {10, 10}}, {{10, 10}, {12, 12}}, {{12, 12}, { 9, 12}},
          {{ 8, 15}, { 9, 13}}, {{ 9, 13}, {11, 15}}, {{11, 15}, { 8, 15}},
          {{ 0, 12}, { 3, 13}}, {{ 3, 13}, { 7, 15}}, {{ 7, 15}, { 8, 16}},
          {{ 2, 16}, { 3, 13}}, {{ 4, 16}, { 5, 14}}, {{ 6, 16}, { 7, 15}},
          {{ 0,  0}, { 8,  0}}, {{12,  0}, {16,  0}}});
  auto r =
    grid(16, 16,
         {{{ 0, 12}, { 1, 14}}, {{ 0,  8}, { 2, 12}}, {{ 0,  4}, { 5, 10}},
          {{ 0,  0}, { 8,  8}}, {{ 1,  1}, { 4,  0}}, {{ 2,  2}, { 8,  0}},
          {{ 3,  3}, { 8,  2}}, {{ 8,  2}, {12,  0}}, {{ 5,  5}, {12,  3}},
          {{12,  3}, {16,  0}}, {{ 0, 16}, { 2, 12}}, {{ 2, 12}, { 8,  8}},
          {{ 8,  8}, {14,  6}}, {{14,  6}, {16,  4}}, {{ 6, 16}, {11, 10}},
          {{11, 10}, {16,  6}}, {{11, 16}, {12, 12}}, {{12, 12}, {16,  8}},
          {{12, 12}, {16, 16}}, {{13, 13}, {16, 10}}, {{14, 14}, {16, 12}},
          {{15, 15}, {16, 14}}});
  auto s =
    grid(16, 16,
         {{{ 0,  0}, { 4,  2}}, {{ 4,  2}, { 8,  2}}, {{ 8,  2}, {16,  0}},
          {{ 0,  4}, { 2,  1}}, {{ 0,  6}, { 7,  4}}, {{ 0,  8}, { 8,  6}},
          {{ 0, 10}, { 7,  8}}, {{ 0, 12}, { 7, 10}}, {{ 0, 14}, { 7, 13}},
          {{ 8, 16}, { 7, 13}}, {{ 7, 13}, { 7,  8}}, {{ 7,  8}, { 8,  6}},
          {{ 8,  6}, {10,  4}}, {{10,  4}, {16,  0}}, {{10, 16}, {11, 10}},
          {{10,  6}, {12,  4}}, {{12,  4}, {12,  7}}, {{12,  7}, {10,  6}},
          {{13,  7}, {15,  5}}, {{15,  5}, {15,  8}}, {{15,  8}, {13,  7}},
          {{12, 16}, {13, 13}}, {{13, 13}, {15,  9}}, {{15,  9}, {16,  8}},
          {{13, 13}, {16, 14}}, {{14, 11}, {16, 12}}, {{15,  9}, {16, 10}}});

  auto above = [&](auto p1, auto p2) {
    return below(p2, p1);
  };
  auto quartet = [&](auto p1, auto p2, auto p3, auto p4) {
    return above(beside(p1, p2), beside(p3, p4));
  };
  auto nonet = [&](auto p1, auto p2, auto p3, auto p4, auto p5,
                  auto p6, auto p7, auto p8, auto p9) {
    return above3(beside3(p1, p2, p3),
                  beside3(p4, p5, p6),
                  beside3(p7, p8, p9));
  };
  auto rot = rotate90;
  auto cycle = [&](auto p1) {
    return quartet(p1, rot(rot(rot(p1))), rot(p1), rot(rot(p1)));
  };
  auto b = white();
  auto t = quartet(p, q, r, s);
  auto side1 = quartet(b, b, rot(t), t);
  auto side2 = quartet(side1, side1, rot(t), t);
  auto u = cycle(rot(q));
  auto corner1 = quartet(b, b, b, u);
  auto corner2 = quartet(corner1, side1, rot(side1), u);
  auto corner = nonet(corner2, side2, side2,
                      rot(side2), u, rot(t),
                      rot(side2), rot(t), q);

  return cycle(corner);
}

// missing; paint.
