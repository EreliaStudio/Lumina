\page lumina_textures Textures

# Textures

Declare 2D textures at top-level:
```cpp
Texture albedo;
```

Use helper methods to sample or convert coordinates. Typical fragment usage:
```cpp
FragmentPass()
{
    Vector2 uv = /* ... */;
    pixelColor = albedo.getPixel(uv);
}
```
