#include <stdio.h>

#include "../pictie.h"

int main (int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s OUT\n", argv[0]);
    return 1;
  }
  
  DrawingContext cx(200);
  auto bg = below(black(), gray());
  auto row = beside3(bg, flipVert(bg), bg);
  auto rows = above3(row, flipVert(row), row);
  paint(cx, rows);

  if (!cx.writePPM(argv[1])) {
    return 1;
  }

  fprintf(stdout, "wrote output to %s\n", argv[1]);
  return 0;
}
