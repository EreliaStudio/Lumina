\page lumina_language_guide Language guide

# Language guide

## Files and includes
Use precompiled or user files:
```cpp
#include "<predefinedInclude>"
#include "path/to/file.lum"
```

## Comments
Single-line `//` and multi-line `/* ... */` are supported.

## Types
Scalar, vector, and matrix types mirror GLSL. See *Built-in types and functions* for full lists.

## Arrays
Declare fixed-size arrays with `Type name[size]`. Array literals use `{}` and must match the element type and size.
```cpp
int values[4] = { 1, 2, 3, 4 };
Color palette[3] = { Color(1, 0, 0, 1), Color(0, 1, 0, 1), Color(0, 0, 1, 1) };
pixelColor = palette[1];
```
Only one-dimensional arrays are supported.

## Pipeline flow
Declare stage-to-stage variables:
```cpp
Input -> VertexPass: Vector3 position;
VertexPass -> FragmentPass: Vector2 uv;
FragmentPass -> Output: Vector4 pixelColor;
```

## Functions
Standard C/GLSL-like functions with return type and parameters:
```cpp
Vector4 toneMap(Vector4 c) { /* ... */ return c; }
```

## Stages
Exactly two functions must exist:
```cpp
VertexPass()     { /* must set pixelPosition */ }
FragmentPass()   { }
```
