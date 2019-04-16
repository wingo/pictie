#include <stdio.h>

#include "../pictie.h"

int main (int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s OUT\n", argv[0]);
    return 1;
  }
  
  DrawingContext cx(500);

  paint(cx,
        path({Vector(.1,.9), Vector(.8,.9), Vector(.1,.2), Vector(.9,.3)},
             Color::black(), 0.01, LineCapStyle::Butt));

  if (!cx.writePPM(argv[1])) {
    return 1;
  }

  fprintf(stdout, "wrote output to %s\n", argv[1]);
  return 0;
}
