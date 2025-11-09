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
