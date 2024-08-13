# Build-In features

## Standard Types
Each of the standard type respect the standard C++ data type for the scalar type, and the Sparkle type for vector and matrix types.
### Scalar Types
- `float`: Single precision floating-point number.
- `int`: Signed integer.
- `uint`: Unsigned integer.
- `bool`: Boolean type.

### Vector Types
All those vectors support swizzability.
- `Vector2`: Vector of 2 single precision floating-point numbers.
- `Vector3`: Vector of 3 single precision floating-point numbers.
- `Vector4`: Vector of 4 single precision floating-point numbers.
- `Vector2Int`: Vector of 2 signed integers.
- `Vector3Int`: Vector of 3 signed integers.
- `Vector4Int`: Vector of 4 signed integers.
- `Vector2UInt`: Vector of 2 unsigned integers.
- `Vector3UInt`: Vector of 3 unsigned integers.
- `Vector4UInt`: Vector of 4 unsigned integers.

### Matrix Types
- `Matrix2x2`: 2x2 matrix of single precision floating-point numbers.
- `Matrix3x3`: 3x3 matrix of single precision floating-point numbers.
- `Matrix4x4`: 4x4 matrix of single precision floating-point numbers.

## Functions

### raiseException
- Use `raiseException` to throw exceptions with custom error messages.
- You can customize your error message using values from the shader
```cpp
raiseException("Error message");
raiseException("Error : myValue(" + myValue + ") != myExpectedValue(" + myExpectedValue + ")");
```

## Structures

### Size
- Define a size, represented by a width and a height
- Attributes:
    - `type mySize.width`
    - `type mySize.height`
- Instances:
    - Size : a size with float as type
    - SizeInt : a size with int as type
    - SizeUInt : a size with uint as type

### Color
- Define a color, as a 4 components RGBA. This component support swizzability
- Attributes:
    - `float r` : A float representing the red component, express from 0 to 1
    - `float g` : A float representing the blue component, express from 0 to 1
    - `float b` : A float representing the green component, express from 0 to 1
    - `float a` : A float representing the transparency component, express from 0 to 1

### Texture
- Define a texture with methods for manipulation and querying.
- Attributes:
    - `SizeInt textureName.size` : The size of the texture, in pixel
