# Pictie: Simple functional geometry

Pictie is a simple, standalone C++ graphics package implementing an
algebra of painters.  It was created not to be a great graphics package
but rather to be a test-bed for compiling C++ libraries to WebAssembly.

![Square Limit](https://raw.githubusercontent.com/wingo/pictie/master/tests/test-escher.png)

## WebAssembly

Coming soon!

## History

Pictie is inspired by Peter Henderson's "Functional Geometry"
([1982](http://pmh-systems.co.uk/phAcademic/papers/funcgeo.pdf),
[2002](https://eprints.soton.ac.uk/257577/1/funcgeo2.pdf)).  "Functional
Geometry" inspired the [Picture
language](https://sarabander.github.io/sicp/html/2_002e2.xhtml#g_t2_002e2_002e4)
from the well-known *Structure and Interpretation of Computer Programs*
computer science textbook.

Pictie takes its functional geometry implementation from Michael Sperber
and Jens Axel Søgaard's [port of the picture
language](https://docs.racket-lang.org/sicp-manual/SICP_Picture_Language.html)
([source](https://github.com/sicp-lang/sicp/blob/master/sicp-pict/main.rkt)).

The simple rasterizing canvas implementation in Pictie was written from
scratch for the project.

## License

The pictie example code is available
[Emscripten's licensing
terms](https://emscripten.org/docs/introducing_emscripten/emscripten_license.html),
which is a permissive free software license.  See [the LICENSE
file](https://github.com/wingo/pictie/blob/master/LICENSE) for full details.
