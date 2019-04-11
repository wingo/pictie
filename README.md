# Pictie: Simple functional geometry

Pictie is a simple, standalone C++ graphics package implementing an
algebra of painters.  It was created not to be a great graphics package
but rather to be a test-bed for compiling C++ libraries to WebAssembly.

![Square Limit](https://raw.githubusercontent.com/wingo/pictie/master/tests/test-escher.png)

## WebAssembly

Coming soon!

## History

Pictie is inspired by Peter Henderson's *Functional Geometry* ([original
1982 paper](http://pmh-systems.co.uk/phAcademic/papers/funcgeo.pdf),
[updated 2002 paper](https://eprints.soton.ac.uk/257577/1/funcgeo2.pdf).
*Functional Geometry* inspired the [Picture
language](https://sarabander.github.io/sicp/html/2_002e2.xhtml#g_t2_002e2_002e4)
from the well-known *Structure and Interpretation of Computer Programs*
computer science textbook.

Pictie takes its functional geometry implementation from
[Racket](https://racket-lang.org/), via [its port of the picture
language](https://docs.racket-lang.org/sicp-manual/SICP_Picture_Language.html)
([source](https://github.com/sicp-lang/sicp/blob/master/sicp-pict/main.rkt)).

The canvas implementation in Pictie was written from scratch for the project.

## License

The pictie example code is currently under the GNU Lesser General Public
License, version 3.0.  I am working on getting it licensed under
something more permissive, but in any case any eventual WebAssembly
integrations will be licensed under something more permissive.
