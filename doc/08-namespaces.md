\page lumina_namespaces Namespaces

# Namespaces

Organize symbols:
```cpp
namespace Lighting
{
    Vector3 diffuse(Vector3 n, Vector3 l) { return max(dot(n,l), 0.0) * l; }

    namespace PBR
    {
        float D_GGX(float NdotH, float a) { /* ... */ return 0.0; }
    }
}
```

Use with `Namespace::Symbol` syntax.
