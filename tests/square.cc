#include <stdio.h>

#include "../pictie.h"

int main (int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s OUT\n", argv[0]);
    return 1;
  }
  
  DrawingContext cx(200);
  PainterPtr square = transform(color(Color(100, 100, 200)),
                                Vector(0.1,0.1),
                                Vector(0.9,0.1),
                                Vector(0.1,0.9));
  paint(cx, square);

  if (!cx.writePPM(argv[1])) {
    return 1;
  }

  fprintf(stdout, "wrote output to %s\n", argv[1]);
  return 0;
}
