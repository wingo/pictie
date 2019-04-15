CFLAGS=-Wall -std=c++17
CC=gcc
CXX=g++

EMSCRIPTEN=/usr/bin
EMXX=$(EMSCRIPTEN)/em++
EMCC=$(EMSCRIPTEN)/emcc
EMRUN=$(EMSCRIPTEN)/emrun

all: pictie.html

pictie.html: pictie.cc pictie.h pictie.bindings.cc
	$(EMXX) --bind -s WASM=1 $(CFLAGS) -o pictie.html pictie.cc pictie.bindings.cc

run: pictie.html
	$(EMRUN) pictie.html

clean:
	rm -f pictie.js pictie.wasm pictie.html pictie.wasm.map
