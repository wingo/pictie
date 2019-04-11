#include <stdio.h>
#include "pictie.h"

DrawingContext::DrawingContext(uint32_t resolution)
  : width_(resolution), height_(resolution),
    canvasFrame_(Vector(0,resolution),
                 Vector(resolution,0),
                 Vector(0,-resolution)),
    pixels_(new Color[resolution * resolution]) {
  fill(Color::white());
}
  
void DrawingContext::fill(const Color& color) {
  for (uint32_t y = 0; y < height_; y++)
    for (uint32_t x = 0; x < width_; x++)
      pixels_[y * width_ + x] = color;
}

static bool rightOf(const Vector &a, const Vector &b, const Vector &c) {
  return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) > 0;
}

static bool insideTriangle(const Vector& a, const Vector &b, const Vector &c,
                           const Vector& p) {
  return rightOf(a, b, p) && rightOf(b, c, p) && rightOf(c, a, p);
}

void DrawingContext::drawTriangle(const Vector& a, const Vector& b,
                                  const Vector& c, const Color& color) {
  const Vector ca = canvasFrame_.project(a);
  const Vector cb = canvasFrame_.project(b);
  const Vector cc = canvasFrame_.project(c);
  for (uint32_t y = 0; y < height_; y++)
    for (uint32_t x = 0; x < width_; x++)
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
void DrawingContext::drawPixels(uint32_t pixelWidth, uint32_t pixelHeight,
                                const Color* pixels, const Vector& origin,
                                const Vector& edge1, const Vector& edge2) {
  drawRectangle(origin, origin.add(edge1),
                origin.add(edge1).add(edge2), origin.add(edge2),
                Color::black());
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
  
void ColorPainter::paint(DrawingContext &cx, const Frame& frame) const {
  cx.drawRectangle(frame.origin,
                   frame.origin.add(frame.edge1),
                   frame.origin.add(frame.edge1).add(frame.edge2),
                   frame.origin.add(frame.edge2),
                   color_);
}

void SegmentsPainter::paint(DrawingContext &cx, const Frame& frame) const {
  for (auto segment : segments_)
    cx.drawLine(frame.project(segment.start), frame.project(segment.end),
                color_, width_, lineCapStyle_);
};
                      
ImagePainter* ImagePainter::fromPPM(const char *fname) {
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

void ImagePainter::paint(DrawingContext &cx, const Frame& frame) const {
  cx.drawPixels(width_, height_, pixels_.get(), frame.origin, frame.edge1,
                frame.edge2);
}
                      
void TransformPainter::paint(DrawingContext &cx, const Frame& frame) const {
  painter_->paint(cx, frame.project(frame_));
}

void OverPainter::paint(DrawingContext &cx, const Frame& frame) const {
  first_->paint(cx, frame);
  second_->paint(cx, frame);
}

PainterPtr color(const Color& color) {
  return PainterPtr(new ColorPainter(color));
}

PainterPtr segments(std::vector<Segment> segments, const Color& color,
                    double width, LineCapStyle lineCapStyle) {
  return PainterPtr(new SegmentsPainter(std::move(segments), color, width, lineCapStyle));
}

PainterPtr image(uint32_t width, uint32_t height,
                 std::unique_ptr<Color[]> pixels) {
  return PainterPtr(new ImagePainter(width, height, std::move(pixels)));
}

PainterPtr transform(PainterPtr painter,
                     const Vector& origin, const Vector& corner1,
                     const Vector& corner2) {
  return PainterPtr(new TransformPainter(painter, origin, corner1, corner1));
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
  return over(transform(a, Vector(0,0.5), Vector(1,0.5), Vector(0,1)),
              transform(b, Vector(0,0), Vector(1,0), Vector(0,0.5)));
};

PainterPtr beside3(PainterPtr a, PainterPtr b, PainterPtr c) {
  return over
    (transform(a, Vector(0,2./3.), Vector(1,2./3.), Vector(0,1)),
     over(transform(b, Vector(0,1./3.), Vector(1,1./3.), Vector(0,2/3.)),
          transform(c, Vector(0,0), Vector(1,0), Vector(0,1./3.))));
};

PainterPtr above3(PainterPtr a, PainterPtr b, PainterPtr c) {
  return over
    (transform(a, Vector(0,0), Vector(1./3.,0), Vector(0,1)),
     over(transform(b, Vector(1/3.,0), Vector(2/3.,0), Vector(1/3.,1)),
          transform(b, Vector(2/3.,0), Vector(1,0), Vector(2/3.,1))));
};

PainterPtr black() { return color(Color::black()); }
PainterPtr gray() { return color(Color::gray()); }
PainterPtr white() { return color(Color::white()); }

PainterPtr vects(std::vector<Vector> vects) {
  std::vector<Segment> segs;
  for (size_t i = 0; i + 1 < vects.size(); i++)
    segs.push_back(Segment(vects[i], vects[i+1]));
  return segments(segs, Color::black(), 0.01, LineCapStyle::Butt);
}

PainterPtr markOfZorro() {
  return vects({Vector(.1,.9), Vector(.8,.9), Vector(.1,.2), Vector(.9,.3)});
}

// (define diagonal-shading (procedure->painter (λ (x y) (* 100 (+ x y)))))
// (define einstein         (bitmap->painter einstein-file))

PainterPtr grid(double width, double height, std::vector<Segment> segs) {
  Frame f(Vector(0,0), Vector(1/width,0), Vector(1/height,0));
  std::vector<Segment> scaled;
  for (auto s : segs)
    scaled.push_back(f.project(s));
  return segments(scaled, Color::black(), 0.01, LineCapStyle::Butt);
}

PainterPtr escher() {
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

void paint(DrawingContext& cx, const Painter& p) {
  cx.fill(Color::white());
  p.paint(cx, Frame(Vector(0,0), Vector(1,0), Vector(0,1)));
}
