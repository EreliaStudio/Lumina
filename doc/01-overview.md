\page lumina_overview Overview

# Overview 

Lumina is a GLSL-oriented shader language that keeps GLSL semantics while adding a structured, compiler-checked surface:
- A **pipeline flow** declares how data moves: `Input -> VertexPass -> FragmentPass -> Output`.
- Two required entry points: `VertexPass()` and `FragmentPass()`.
- **Structures**, **DataBlocks** (declared as constant or attribute), **Textures**, **Namespaces** mirror familiar C++/GLSL patterns.
- A **compiled artifact** accompanies the generated GLSL and enumerates layouts, framebuffers, blocks, and textures Sparkle can consume.

This lets you write concise shader scripts while Sparkle handles binding, reflection data, and validation.
