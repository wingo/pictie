#include <stdio.h>

#include "../pictie.h"

static PainterPtr vects(std::vector<Vector> vects) {
  std::vector<Segment> segs;
  for (size_t i = 0; i + 1 < vects.size(); i++)
    segs.push_back(Segment(vects[i], vects[i+1]));
  return segments(segs, Color::black(), 0.01, LineCapStyle::Square);
}

int main (int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s OUT\n", argv[0]);
    return 1;
  }
  
  DrawingContext cx(500);

  paint(cx,
        vects({Vector(.1,.9), Vector(.8,.9), Vector(.1,.2), Vector(.9,.3)}));

  if (!cx.writePPM(argv[1])) {
    return 1;
  }

  fprintf(stdout, "wrote output to %s\n", argv[1]);
  return 0;
}
