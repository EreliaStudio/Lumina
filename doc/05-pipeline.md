\page lumina_pipeline Pipeline flow

# Pipeline flow

Valid flows:
- `Input -> VertexPass`
- `VertexPass -> FragmentPass`
- `FragmentPass -> Output`

Data must be native types (scalars, vectors, matrices). Define one declaration per line:
```cpp
Input -> VertexPass: Vector3 position;
VertexPass -> FragmentPass: Vector3 normal;
FragmentPass -> Output: Vector4 color;
```
