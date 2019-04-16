# Example Pictie programs

A painter is a program that paints.  There are primitive painters, that
paint lines or colors, and composite painters, that compose together a
number of other painters.

These examples are made for pasting into the pictie web interface.

Incidentally, it could be that the JavaScript bindings to Pictie could
be more natural.  In Pictie we are trying to have a native C++ library
with idiomatic JS bindings.  Much of the weirdness around the interfaces
(e.g. the need to do `new Color()` to make a color, the lack of
overloading in some situations, the lack of optional arguments) is
related to limitations in the software that generates the JS bindings.
We will be improving this over time.

## Primitive painters

### Triangle

The `triangle` painter paints a triangle, and takes the vertices and the
color as arguments.

```
triangle(new Vector(0.1, 0.1), new Vector(0.2, 0.7), new Vector(0.8, 0.5),
         new Color(120, 200, 50))
```

It would be nice if Color were callable directly, somehow.  It would be
possible to change the bindings to represent Color values as raw arrays
of ints, or as objects like {'r':120,'g':200,'b':50}, but then we would
lose the ability to invoke methods on Color values.

### Path

The `path` painter paints a segmented line.  The C++ function takes the
points in the path as its first argument, as a `std::vector<Vector>`.

Unfortunately the embind bindings for `std::vector` are really hairy.
You have to first declare in the bindings file the set of vector types
you will have, and you have to give them names.  In our case we use
`VectorArray` for a `std::vector<>` of `Vector`.  The constructor is
nullary; you have to then manually push on the values.  It's a bit of a
bummer.

```
let markOfZorro = [[.1,.9], [.8,.9], [.1,.2], [.9,.3]];
let makeVectorArray = points => {
  let ret = new VectorArray();
  for (let p of points)
    ret.push_back(new Vector(p[0], p[1]));
  return ret;
};
path(makeVectorArray(markOfZorro), Color.black(), 0.01, LineCapStyle.Butt,
     LineWidthScaling.Unscaled)
```

The other arguments to `path` are the line color, width, cap style, and
whether the line width should scale when/if the painter is transformed.

### Image

The `image` painter paints some pixels.  It has a similar problem that
`path` has in that it takes the pixels as a `std::vector<Color>`, a
`ColorArray`.

```
let crab=[16,1,5,1,7,1,3,1,7,7,5,2,1,3,1,2,3,11,2,1,1,7,1,1,2,1,1,1,5,1,1,1,5,2,1,2,17];
let pixels=new ColorArray;
let colors=[Color.black(), new Color(50, 50, 220)];
for (let i=0, c=0; i<crab.length; i++, c++) {
  for (let j=0; j<crab[i]; j++)
    pixels.push_back(colors[c%colors.length]);
}
image(13, 10, pixels)
```

The first two arguments are the pixel width and height, respectively.

## Basic composition primitives

Pictie provides two basic composition primitives.

### Transform

Return a painter that paints within a different bounding box, specified
as an origin point and two corners, indicating the ends of the X and Y
axes, respectively.

```
let t = triangle(new Vector(0.1, 0.1), new Vector(0.2, 0.7),
                 new Vector(0.8, 0.5), new Color(120, 200, 50));
transform(t, new Vector(0, 1), new Vector(1, 1), new Vector(0,0));
```

### Over

Return a painter that paints one painter on top of another.

```
let t = triangle(new Vector(0.1, 0.1), new Vector(0.2, 0.7),
                 new Vector(0.8, 0.5), new Color(120, 200, 50));
let u = triangle(new Vector(0.1, 0.7), new Vector(0.7, 1.0),
                 new Vector(0.3, 0.3), new Color(200, 120, 50));
over(t, u)
```

## Derived painters

### Parallelogram

Draw a parallelogram.

```
let origin = new Vector(0.2, 0.1);
let edge1 = new Vector(0.5, 0.1);
let edge2 = new Vector(0.1, 0.4);
parallelogram(origin, edge1, edge2, new Color(10, 50, 100))
```

### Flip

Flip a painter horizontally or vertically.  If `$0` is a painter, then
these do what you'd expect:

```
flipHoriz($0)
```

```
flipVert($0)
```

### Rotate

Rotate a painter clockwise (?).  If `$0` is a painter, then these do
what you'd expect:

```
rotate90($0)
```

```
rotate180($0)
```

```
rotate270($0)
```

### Compose

Divide the canvas into regions and paint each painter into the region
provided.

```
beside($0, $1)
```

```
below($0, $1)
```

```
beside3($0, $1, $2)
```

```
above3($0, $1, $2)
```

### Color fills

Fill the unit square with the given color.

```
black()
```

```
gray()
```

```
white()
```

## Painter interface

See pictie.html for the interface for how to actually set up a paint and
get pixels into a canvas.

Note that for the moment we don't actually clean up after anything.  It
would be hard to do for the painters, as they are shared_ptr instances.
But for the `DrawingContext`, we should probably delete it promptly when
we get the pixels out.  But in the short term we plan to fix emscripten
so that it will hook up finalizers.

See also the
[embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html)
file,
[`pictie.bindings.cc`](https://github.com/wingo/pictie/blob/master/pictie.bindings.cc).
