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

It's easier than you might think!

### Install Emscripten

This is probably not the easiest way to install Emscripten, but since
Emscripten just switched to use LLVM's WebAssembly backend instead of
its own, at least it is foolproof.  See [the upstream
documentation](https://emscripten.org/docs/getting_started/downloads.html)
for the official recommendations.

Download and build LLVM, enabling WebAssembly and the LLVM linker:

```
cd ~/src
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build
cd build
cmake -DLLVM_ENABLE_PROJECTS='lld;clang' -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -G "Unix Makefiles" ../llvm
make -j40
```

You might have to adjust the `-j40` level.  Also, the final linking
phase sometimes runs out of memory, so you might need to re-run the
`make` stage until it works.

Then download Emscripten:

```
cd ~/src
git clone https://github.com/emscripten-core/emscripten.git
```

Now Emscripten is mostly written in Python actually, so you don't need
to build it.  Just run `~/src/emscripten/emcc` and it will set itself up
for the first time, creating a file `~/.emscripten` as it does so.

You need to edit that file to point it to your LLVM.  Go into `~/.emscripten` and
change the `LLVM_ROOT` line to read:

```
LLVM_ROOT = os.path.expanduser(os.getenv('LLVM', '~/src/llvm-project/build/bin'))
```

And at that point, you should be good to go.

### Build pictie.wasm

If you installed emscripten as above, then to build WebAssembly files,
just run:

```
make EMSCRIPTEN=~/src/emscripten
```

If you installed emscripten somewhere else, adjust `EMSCRIPTEN`
appropriately.

The result will be a `pictie.js` and `pictie.wasm` file in the local
directory.

The default optimization flags passed to Emscripten is `-Oz`, to
optimize for smallest code size.  You can change these flags by setting
`EMFLAGS`, as in `make EMSCRIPTEN=~/src/emscripten EMFLAGS='-Oz
--closure 1'`.  (Setting `--closure 1` enables the use of the Google
closure compiler on `pictie.js`, for better dead-code elimination and
minification.)

### Test it out in your web browser

There is a `pictie.html` test harness in the root directory that
provides a kind of interactive graphics workbench, and which we use to
test out different modifications to the emscripten toolchain.  This
requires that the web browser can be able to load `pictie.js` and
`pictie.wasm` from the same directory, which isn't always the case;
Chrome for example will refuse to do so.  So, the solution there is that
emscripten packages a web server as well, which you can run via `make
run`.  As before, you might need to pass `EMSCRIPTEN=~/src/emscripten`
or something on the command-line.

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
