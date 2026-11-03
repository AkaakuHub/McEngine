# OpenGL depth probe

The Windows-only probe opens an invisible OpenGL context, reports the driver's actual renderbuffer depth precision, and checks distant 2D overlay composition against a closer scene occluder. The perspective range is 0.01-300 metres; the two overlay layers are at 25 and 25.0005 metres. Depth-writing layers lose the later layer, while disabling only their depth writes preserves painter order and retains the scene's depth occlusion.

Build with a Windows C++11 compiler and link `opengl32` and `gdi32`:

```
g++ -std=c++11 tools/depth-probe/main.cpp -o depth-probe.exe -lopengl32 -lgdi32
```

The test exits nonzero if the overlay/occluder checks fail. It does not demonstrate HMD output or measure flicker on a headset.
