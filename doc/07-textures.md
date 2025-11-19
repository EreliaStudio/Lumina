\page lumina_textures Textures

# Textures

Declare 2D textures at top-level. Textures default to the **constant** scope, meaning the same binding is shared across every draw that uses the shader. Use the `as` clause to override the scope on a per-texture basis:
```cpp
Texture albedoTexture;              // implicit constant scope
Texture normalTexture as attribute; // unique per render item
Texture emissiveTexture as constant;
```

Available scopes:
- `constant`: shared by all draw calls that use the shader (ideal for frame-level resources such as environment maps or LUTs).
- `attribute`: treated like an attribute block entry and expected to change per render submission (e.g., per material or per mesh).

Use helper methods to sample or convert coordinates. Typical fragment usage:
```cpp
FragmentPass()
{
    Vector2 uv = /* ... */;
    pixelColor = albedo.getPixel(uv);
}
```
