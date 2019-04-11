#include <stdio.h>

#include "../pictie.h"

int main (int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s OUT\n", argv[0]);
    return 1;
  }
  
  DrawingContext cx(800);

  auto nonet = [&](auto p1, auto p2, auto p3, auto p4, auto p5,
                   auto p6, auto p7, auto p8, auto p9) {
    return above3(beside3(p1, p2, p3),
                  beside3(p4, p5, p6),
                  beside3(p7, p8, p9));
  };
  paint(cx, nonet(color(Color(200,50,50)),
                  color(Color(50,200,50)),
                  color(Color(50,50,200)),
                  color(Color(200,100,100)),
                  color(Color(100,200,100)),
                  color(Color(100,100,200)),
                  color(Color(200,150,150)),
                  color(Color(150,200,150)),
                  color(Color(150,150,200))));

  if (!cx.writePPM(argv[1])) {
    return 1;
  }

  fprintf(stdout, "wrote output to %s\n", argv[1]);
  return 0;
}
