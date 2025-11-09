\page lumina Lumina

Lumina is Sparkle's custom GLSL-script language for writing shaders. It compiles to GLSL plus a machine-readable artifact that Sparkle uses to bind inputs, outputs, constants, attributes, and textures.

- **Use in Sparkle**: For shaders, Sparkle uses the custom GLSL-script **Lumina**. Documentation is here.
- **What you write**: a `.lum` file describing pipeline flow, types, blocks, functions, and the two stages `VertexPass()` and `FragmentPass()`.
- **What Sparkle needs**: the generated GLSL and the compiled artifact with layout, framebuffer, blocks, and textures.

# Summary
- \subpage lumina_overview "Overview"
- \subpage lumina_getting_started "Getting started"
- \subpage lumina_language_guide "Language guide"
- \subpage lumina_builtins "Built-in types and functions"
- \subpage lumina_pipeline "Pipeline flow"
- \subpage lumina_structures_blocks "Structures and blocks"
- \subpage lumina_textures "Textures"
- \subpage lumina_namespaces "Namespaces"
- \subpage lumina_artifact "Compiled artifact format"
- \subpage lumina_cheatsheet "Cheat sheet"

---
