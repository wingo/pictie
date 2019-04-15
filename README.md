# Pictie: Simple functional geometry

Pictie is a simple, standalone C++ graphics package implementing an
algebra of painters.  It was created not to be a great graphics package
but rather to be a test-bed for compiling C++ libraries to WebAssembly.

![Square Limit](https://raw.githubusercontent.com/wingo/pictie/master/tests/test-escher.png)

## Testing

Run `make -C tests check` to build pictie "natively" and use it to
generate some test images.  The reference outputs are checked into
revision control and can be viewed [on
github](https://github.com/wingo/pictie/tree/master/tests).

## WebAssembly

It's easier than you might think!  If Emscripten is in your path, then
you just run `make`.  The result will be a `pictie.js` and `pictie.wasm`
file in the local directory.

If you have Emscripten built in `~/src/emscripten-core/emscripten`, then
pass the `EMSCRIPTEN` variable to make, as in `make
EMSCRIPTEN=~/src/emscripten-core/emscripten`.

There is a `pictie.html` test harness in the root directory that
provides a kind of interactive graphics workbench, and which we use to
test out different modifications to the emscripten toolchain.  This
requires that the web browser can be able to load `pictie.js` and
`pictie.wasm` from the same directory, which isn't always the case;
Chrome for example will refuse to do so.  So, the solution there is that
emscripten packages a web server as well, which you can run via `make
run`.  As before, you might need to pass
`EMSCRIPTEN=~/src/emscripten-core/emscripten` or something on the
command-line.

But, maybe you just want to have a peek, so you can give it a go at
https://people.igalia.com/awingo/pictie.html.  No guarantees that this
copy will be kept up to date though!

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
