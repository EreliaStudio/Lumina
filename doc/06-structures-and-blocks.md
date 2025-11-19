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

## DataBlock
Uniform-style data described once and consumed by both stages. Declare a `DataBlock` and choose its scope with an optional `as` clause:
```cpp
DataBlock SceneConstants            // defaults to constant scope
{
    Vector3 sunDirection;
    float exposure;
};

DataBlock ModelData as attribute    // per render submission
{
    Matrix4x4 model;
    Matrix4x4 normal;
};
```
- `constant` (default): the runtime binds the same data for every draw call that uses the shader (think camera or lighting parameters).
- `attribute`: the runtime expects a distinct binding for each submission (similar to per-instance material data).
