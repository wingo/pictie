CFLAGS=-Wall -std=c++17
CC=gcc
CXX=g++

EMSCRIPTEN=/usr/bin
EMXX=$(EMSCRIPTEN)/em++
EMCC=$(EMSCRIPTEN)/emcc
EMRUN=$(EMSCRIPTEN)/emrun

all: pictie.js

pictie.js: pictie.cc pictie.h pictie.bindings.cc
	$(EMXX) --bind -s ENVIRONMENT=web -s WASM=1 $(CFLAGS) -o pictie.js pictie.cc pictie.bindings.cc

run: pictie.js
	$(EMRUN) pictie.html

clean:
	rm -f pictie.js pictie.wasm pictie.wasm.map
