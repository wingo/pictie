#include <stdio.h>

#include "../pictie.h"

int main (int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s OUT\n", argv[0]);
    return 1;
  }
  
  DrawingContext cx(800);

  auto above = [&](auto p1, auto p2) {
    return below(p2, p1);
  };
  auto quartet = [&](auto p1, auto p2, auto p3, auto p4) {
    return above(beside(p1, p2), beside(p3, p4));
  };
  paint(cx, quartet(color(Color(200,100,100)),
                    color(Color(100,200,100)),
                    color(Color(100,100,200)),
                    color(Color(100,100,100))));

  if (!cx.writePPM(argv[1])) {
    return 1;
  }

  fprintf(stdout, "wrote output to %s\n", argv[1]);
  return 0;
}
