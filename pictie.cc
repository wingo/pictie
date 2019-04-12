#include <stdio.h>
#include "pictie.h"

DrawingContext::DrawingContext(uint32_t resolution)
  : width_(resolution), height_(resolution),
    canvasFrame_(Vector(0,resolution),
                 Vector(resolution,0),
                 Vector(0,-double(resolution))),
    pixels_(new Color[resolution * resolution]) {
  fill(Color::white());
}
  
void DrawingContext::fill(const Color& color) {
  for (uint32_t y = 0; y < height_; y++)
    for (uint32_t x = 0; x < width_; x++)
      pixels_[y * width_ + x] = color;
}

static bool rightOf(const Vector &a, const Vector &b, const Vector &c) {
  return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0;
}

static bool insideTriangle(const Vector& a, const Vector &b, const Vector &c,
                           const Vector& p) {
  return rightOf(a, b, p) && rightOf(b, c, p) && rightOf(c, a, p);
}

template<typename T>
static T min(T a, T b) { return a < b ? a : b; }
template<typename T>
static T max(T a, T b) { return a > b ? a : b; }
  
template<typename T>
static T min(T a, T b, T c) { return min(a, min(b, c)); }
template<typename T>
static T max(T a, T b, T c) { return max(a, max(b, c)); }
template<typename T>
static T min(T a, T b, T c, T d) { return min(min(a, b), min(c, d)); }
template<typename T>
static T max(T a, T b, T c, T d) { return max(max(a, b), max(c, d)); }
  
template<typename T>
static T clamp(T lo, T x, T hi) { return max(lo, min(x, hi)); }

void DrawingContext::drawTriangle(const Vector& a, const Vector& b,
                                  const Vector& c, const Color& color) {
  const Vector ca = canvasFrame_.project(a);
  const Vector cb = canvasFrame_.project(b);
  const Vector cc = canvasFrame_.project(c);
  uint32_t miny = floor(clamp(0.0, min(ca.y, cb.y, cc.y), double(height_)));
  uint32_t maxy = ceil(clamp(0.0, max(ca.y, cb.y, cc.y), double(height_)));
  uint32_t minx = floor(clamp(0.0, min(ca.x, cb.x, cc.x), double(width_)));
  uint32_t maxx = ceil(clamp(0.0, max(ca.x, cb.x, cc.x), double(width_)));

  for (uint32_t y = miny; y < maxy; y++)
    for (uint32_t x = minx; x < maxx; x++)
      if (insideTriangle(ca, cb, cc, Vector(double(x) + 0.5, double(y) + 0.5)))
        pixels_[y * width_ + x] = color;
}

void DrawingContext::drawRectangle(const Vector& a, const Vector &b,
                                   const Vector &c, const Vector& d,
                                   const Color& color) {
  drawTriangle(a, b, c, color);
  drawTriangle(c, d, a, color);
}
    
void DrawingContext::drawLine(const Vector& a, const Vector &b,
                              const Color& color, double width,
                              LineCapStyle lineCapStyle) {
  Vector q = b.sub(a).normalize(width * 0.5);
  Vector r = q.rotate90DegreesClockwise();
    
  switch (lineCapStyle) {
  case LineCapStyle::Butt:
    drawRectangle(a.add(r), b.add(r), b.sub(r), a.sub(r), color);
    break;
  case LineCapStyle::Square:
    drawRectangle(a.sub(q).add(r), b.add(q).add(r), b.add(q).sub(r),
                  a.sub(q).sub(r),
                  color);
    break;
  }
}
    
void DrawingContext::drawPixels(uint32_t pixelWidth, uint32_t pixelHeight,
                                const Color* pixels, const Vector& origin,
                                const Vector& edge1, const Vector& edge2) {
  Vector dx = edge1.scale(1./pixelWidth);
  Vector dy = edge2.scale(1./pixelHeight);
  Vector row = origin;
  for (uint32_t y = 0; y < pixelHeight; y++, row = row.add(dy)) {
    Vector p = row;
    for (uint32_t x = 0; x < pixelWidth; x++, p = p.add(dx))
      drawRectangle(p, p.add(dx), p.add(dx).add(dy), p.add(dy),
                    pixels[(pixelHeight - y - 1) * pixelWidth + x]);
  }
}

bool DrawingContext::writePPM(const char *fname) {
  FILE *f = fopen(fname, "w");
  if (!f) {
    fprintf(stderr, "failed to create %s: ", fname);
    perror(NULL);
    return false;
  }
  fprintf(f, "P6\n%u %u\n255\n", width_, height_);
  for (uint32_t y = 0; y < height_; y++) {
    for (uint32_t x = 0; x < width_; x++) {
      const Color& p = pixels_[y * width_ + x];
      fprintf(f, "%c%c%c", p.r, p.g, p.b);
    }
  }
  if (fflush(f) != 0) {
    perror("writing PPM file failed"); 
    fclose(f);
    return false;
  }
  fclose(f);
  return true;
}
  
class ColorPainter : public Painter
{
private:
  const Color color_;

public:
  explicit ColorPainter(const Color& color) : color_(color) {}
  void paint(DrawingContext &cx, const Frame& frame) const {
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
  LineWidthScaling widthScaling_;

public:
  SegmentsPainter(std::vector<Segment> segments, const Color& color,
                  double width = 1.0,
                  LineCapStyle lineCapStyle = LineCapStyle::Butt,
                  LineWidthScaling widthScaling = LineWidthScaling::Unscaled)
    : segments_(segments), color_(color), width_(width),
      lineCapStyle_(lineCapStyle), widthScaling_(widthScaling) {}

  void paint(DrawingContext &cx, const Frame& frame) const {
    double width;
    switch (widthScaling_) {
    case LineWidthScaling::Scaled: {
      double diagonal = frame.edge1.add(frame.edge2).magnitude();
      double unitDiagonal = Vector(1,1).magnitude();
      width = width_ * diagonal / unitDiagonal;
      break;
    }
    case LineWidthScaling::Unscaled:
      width = width_;
      break;
    default:
      abort();
    }
    for (auto segment : segments_)
      cx.drawLine(frame.project(segment.start), frame.project(segment.end),
                  color_, width, lineCapStyle_);
  };
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

  void paint(DrawingContext &cx, const Frame& frame) const {
    cx.drawPixels(width_, height_, pixels_.get(), frame.origin, frame.edge1,
                  frame.edge2);
  }
};
                      
class TransformPainter : public Painter
{
private:
  PainterPtr painter_;
  const Frame frame_;

public:
  TransformPainter(PainterPtr painter,
                   const Vector& origin, const Vector& corner1,
                   const Vector& corner2)
    : painter_(painter),
      frame_(origin, corner1.sub(origin), corner2.sub(origin))
  {}
  
  void paint(DrawingContext &cx, const Frame& frame) const {
    painter_->paint(cx, frame.project(frame_));
  }

};

class OverPainter : public Painter
{
private:
  PainterPtr first_;
  PainterPtr second_;

public:
  OverPainter(PainterPtr first, PainterPtr second)
    : first_(first), second_(second) {}
  
  void paint(DrawingContext &cx, const Frame& frame) const {
    first_->paint(cx, frame);
    second_->paint(cx, frame);
  }
};

PainterPtr color(const Color& color) {
  return PainterPtr(new ColorPainter(color));
}

PainterPtr segments(std::vector<Segment> segments, const Color& color,
                    double width, LineCapStyle lineCapStyle,
                    LineWidthScaling widthScaling) {
  return PainterPtr(new SegmentsPainter(std::move(segments), color, width,
                                        lineCapStyle, widthScaling));
}

PainterPtr image(uint32_t width, uint32_t height,
                 std::unique_ptr<Color[]> pixels) {
  return PainterPtr(new ImagePainter(width, height, std::move(pixels)));
}

PainterPtr transform(PainterPtr painter,
                     const Vector& origin, const Vector& corner1,
                     const Vector& corner2) {
  return PainterPtr(new TransformPainter(painter, origin, corner1, corner2));
}

PainterPtr over(PainterPtr a, PainterPtr b) {
  return PainterPtr(new OverPainter(a, b));
}

PainterPtr flipHoriz(PainterPtr painter) {
  return transform(painter, Vector(1, 0), Vector(0, 0), Vector(1, 1));
};

PainterPtr flipVert(PainterPtr painter) {
  return transform(painter, Vector(1, 0), Vector(0, 0), Vector(1, 1));
};

PainterPtr rotate90(PainterPtr painter) {
  return transform(painter,Vector(1, 0), Vector(1, 1), Vector(0, 0));
};

PainterPtr rotate180(PainterPtr painter) {
  return rotate90(rotate90(painter));
};

PainterPtr rotate270(PainterPtr painter) {
  return rotate90(rotate90(rotate90(painter)));
};

PainterPtr beside(PainterPtr a, PainterPtr b) {
  return over(transform(a, Vector(0,0), Vector(0.5,0), Vector(0,1)),
              transform(b, Vector(0.5,0), Vector(1,0), Vector(0.5,1)));
};

PainterPtr below(PainterPtr a, PainterPtr b) {
  return rotate270(beside(rotate90(b), rotate90(a)));
};

PainterPtr beside3(PainterPtr a, PainterPtr b, PainterPtr c) {
  return over
    (transform(a, Vector(0,0), Vector(1/3.,0), Vector(0,1)),
     over(transform(b, Vector(1/3.,0), Vector(2/3.,0), Vector(1/3.,1)),
          transform(c, Vector(2/3.,0), Vector(1,0), Vector(2/3.,1))));
};

PainterPtr above3(PainterPtr a, PainterPtr b, PainterPtr c) {
  return over
    (transform(a, Vector(0,2/3.), Vector(1,2/3.), Vector(0,1)),
     over(transform(b, Vector(0,1/3.), Vector(1,1/3.), Vector(0,2/3.)),
          transform(c, Vector(0,0), Vector(1,0), Vector(0,1./3.))));
};

PainterPtr black() { return color(Color::black()); }
PainterPtr gray() { return color(Color::gray()); }
PainterPtr white() { return color(Color::white()); }

// (define diagonal-shading (procedure->painter (λ (x y) (* 100 (+ x y)))))
// (define einstein         (bitmap->painter einstein-file))

void paint(DrawingContext& cx, const Painter& p) {
  cx.fill(Color::white());
  p.paint(cx, Frame(Vector(0,0), Vector(1,0), Vector(0,1)));
}
