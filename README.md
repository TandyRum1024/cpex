# CPEX: C++ experiments / exercises
---
Small C++ programs, mainly featuring C++, OpenGL and UI powered by ImGui.

# Preface
For the longest time, I've been working with graphics programming and likes in GameMaker game engine; from GM:S 1.4, GM:S 2.X to (now renamed) GameMaker.
GameMaker, being 2D-focused engine (as of writing), had not the most robust 3D features that can be used out-of-the-box. But on the other hand, it provided somewhat low-level graphics API, from vertex buffers and support basic (transform) matrices and shaders.
I had to re-invent many of the features myself, that are normally "activated" with a single click in the 3D-focused game engines such as Godot and Unity.

While the low-level graphics API was just enough to create a functioning 3D renderer, I have faced a few limitations of GameMaker. For example, in context of shaders...
- No texture fetching in vertex shaders.
- Lack of cubemap textures support. May be circumvented with manually converting 3D vector for cubemap samplers into 2D UV coordinates in shader, but emulating GPU interpolations are painful and might cause overhead from the conversion.
- Lack of runtime compilation of shader program.
- Only vertex & fragment/pixel shaders are supported. That is, lack of geometry & compute shaders, etc.

Those limitations have certainly prevented me from implementing some of the techniques. Such as using vertex texture fetching to implement an alternative method of vertex animation, GPU side animation mixing/calculations, etc.

Eventually, from online tutorials and likes for implementing various graphics techniques made me aware of the elusive C++ / OpenGL duo. They are seemingly the de-facto standard when it comes to graphics programming.
So it made me want to try them out aswell; I especially am hoping to not encounter such limitations, and to build up more general knowledge that can be used outside the context of GameMaker engine.

With that being said, this repository is a collection of my "exercises" / "experiments" working with C++.

# Projects summeary
- `CPEX/cpex_t1`: T1, C++ (re)primer and OpenGL basics. Features a simple OpenGL application with controls UI, powered by ImGui