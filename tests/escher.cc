#include <stdio.h>

#include "../pictie.h"

class Segment {
public:
  Vector from, to;
  Segment(Vector from, Vector to) : from(from), to(to) {}
};

static PainterPtr grid(double width, double height, std::vector<Segment> segs) {
  Frame f(Vector(0,0), Vector(1/width,0), Vector(0,1/height));
  auto line = [&](auto a, auto b) {
    return path({f.project(a), f.project(b)}, Color::black(), 0.002,
                LineCapStyle::Square, LineWidthScaling::Unscaled);
  };
    
  PainterPtr ret = nullptr;
  for (auto s : segs) {
    auto p = line(s.from, s.to);
    ret = ret ? over(ret, p) : p;
  }
  return ret;
}

static PainterPtr escher() {
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

int main (int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s OUT\n", argv[0]);
    return 1;
  }
  
  DrawingContext cx(800);

  paint(cx, escher());

  if (!cx.writePPM(argv[1])) {
    return 1;
  }

  fprintf(stdout, "wrote output to %s\n", argv[1]);
  return 0;
}
