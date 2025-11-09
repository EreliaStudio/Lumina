\page lumina_structures_blocks Structures and blocks

# Structures and blocks

## Structures
C++-like aggregates. Support methods, constructors, and operator definitions.

```cpp
struct Material
{
    Vector3 albedo;
    float roughness;

    Vector3 brdf(Vector3 n, Vector3 v, Vector3 l) { /* ... */ return albedo; }
    Material operator+(Material rhs) { return Material(albedo + rhs.albedo, max(roughness, rhs.roughness)); }
};
```

## AttributeBlock
Per-draw-call uniforms shared by vertex and fragment.
```cpp
AttributeBlock Model
{
    Matrix4x4 model;
    Matrix4x4 normal;
};
```

## ConstantBlock
Application-wide constants shared across pipelines.
```cpp
ConstantBlock Globals
{
    Vector3 lightPos;
    float ambient;
};
```
