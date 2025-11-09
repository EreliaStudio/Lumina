\page lumina_getting_started Getting started

# Getting started

## Minimal pipeline

```cpp
// Includes
#include <screenConstants>
#include "shader/customInclude.lum"

// Pipeline flow
Input -> VertexPass: Vector3 vertexPosition;
VertexPass -> FragmentPass: Vector2 fragUV;

// Vertex stage
VertexPass()
{
    Vector4 clip = Vector4(vertexPosition, 1.0);
    pixelPosition = clip;
    fragUV = vertexPosition.xy * 0.5 + 0.5;
}

// Fragment stage
FragmentPass()
{
    Vector4 finalColor = Vector4(fragUV.x, fragUV.y, 0.0, 1.0);
    pixelColor = finalColor;
}
```

## Build outputs

The compiler emits:
- GLSL for vertex and fragment stages.
- A compiled artifact Sparkle reads to bind your pipeline (see *Compiled artifact format*).
