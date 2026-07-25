# OpenToonz Headless

A headless, programmatically controllable fork of [OpenToonz](https://github.com/opentoonz/opentoonz) — the open-source 2D animation software. Exposes the full animation pipeline via a JSON-RPC interface over stdin/stdout, designed for automated workflows.

## Headless Interface

The `toonz_headless` binary provides a JSON-RPC 2.0 protocol for controlling OpenToonz without a GUI. Send one JSON object per line to stdin, read responses from stdout.

```bash
echo '{"id":1,"method":"eval","params":{"code":"print(1+1)"}}' | toonz_headless
# → {"jsonrpc":"2.0","id":1,"result":{"ok":true,"output":"2"}}
```

State persists across eval calls within a session — variables created in one command are available in the next.

### What You Can Do

| Capability | Class | Description |
|-----------|-------|-------------|
| Vector drawing | `Stroke`, `VectorImage` | Create strokes from point sequences, add to images, fill regions |
| Raster drawing | `RasterCanvas` | CPU-based brush strokes and flood fill on indexed-color rasters |
| Color management | `Palette` | Create palettes, add/modify colors, assign to levels |
| Scene assembly | `Scene`, `Level` | Create levels, set frames, organize in XSheet timeline |
| Keyframe animation | `StageObject` | Animate position, rotation, scale with easing curves |
| Auto-inbetweening | `Inbetween` | Generate intermediate vector frames between key drawings |
| Mesh deformation | `PlasticRig` | Build deformation skeletons with animatable vertices |
| Built-in effects | `Effect` | Access 145 effects (particles, blur, glow, etc.) by identifier |
| Rendering | `Rasterizer`, `Renderer` | Rasterize supported image types and render complete scenes |

### Quick Example: Render Vector Art Through a Scene

The most reliable path for vector output is to place the vector image in a level and scene, then render it with `Renderer`.

```javascript
// Create a palette
var pal = new Palette();
var red = pal.addColor(255, 0, 0, 255);
var blue = pal.addColor(0, 80, 255, 255);

// Create visible vector artwork
var vi = new VectorImage();
vi.setPalette(pal);
vi.addFilledRect(-180, -180, 180, 180, 12, red, red);
vi.addFilledRect(-90, -90, 90, 90, 12, blue, blue);

// Put the vector image in a level and scene
var scene = new Scene();
scene.setCameraSize(512, 512);
scene.setFrameRate(24);

var level = scene.newLevel("Vector", "example_vector");
level.setPalette(pal);
level.setFrame(1, vi.toImage());
scene.setCell(0, 0, level, 1);

// Render frame 0 to PNG
var renderer = new Renderer();
var output = renderer.renderFrame(scene, 0);
output.save("/tmp/vector_render.png");
```

#### Direct `Rasterizer` note

In testing, direct rasterization of a `VectorImage` using the pattern below produced a valid but blank PNG, while the same vector image rendered correctly through `Scene` and `Renderer`:

```javascript
var rast = new Rasterizer();
rast.xres = 512;
rast.yres = 512;
rast.dpi = 72;
rast.rasterize(vi.toImage()).save("/tmp/vector.png");
```

Until the direct vector rasterization path is confirmed or corrected, use the `Scene` + `Renderer` workflow above for vector output. `RasterCanvas` output has also been verified to save correctly as colored PNG images.

For the full API reference, see [HEADLESS_API.md](./HEADLESS_API.md).

## Building (Linux)

### Install Dependencies

```bash
sudo apt-get install -y build-essential cmake pkg-config ninja-build \
  libboost-all-dev qtbase5-dev qt5-qmake qtscript5-dev qttools5-dev \
  qttools5-dev-tools qtmultimedia5-dev libqt5svg5-dev libqt5opengl5-dev \
  libqt5multimedia5-plugins libqt5serialport5-dev libsuperlu-dev liblz4-dev \
  libusb-1.0-0-dev liblzo2-dev libpng-dev libjpeg-dev libglew-dev \
  freeglut3-dev libfreetype6-dev libjson-c-dev libmypaint-dev \
  libopencv-dev libturbojpeg-dev libomp-dev zlib1g-dev libopenblas-dev
```

### Build Custom TIFF

```bash
cd thirdparty/tiff-4.0.3
CFLAGS="-fPIC" CXXFLAGS="-fPIC" ./configure --disable-jbig
make -j$(nproc)
cd ../..
```

### Build Headless Binary

```bash
mkdir -p toonz/sources/build && cd toonz/sources/build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja toonz_headless
```

The binary is at `toonz/sources/build/bin/toonz_headless`.

### Run

```bash
echo '{"id":1,"method":"ping","params":{}}' | ./bin/toonz_headless
```

### WSL 2 and display-less environments

The Linux build has been successfully configured, compiled, and exercised under Ubuntu running in WSL 2 on Windows 10. For WSL, CI, containers, and other environments without a graphical display, launch with Qt's offscreen platform:

```bash
QT_QPA_PLATFORM=offscreen ./bin/toonz_headless
```

A complete smoke test can send `ping` followed by a clean shutdown:

```bash
printf '%s\n' \
  '{"id":1,"method":"ping","params":{}}' \
  '{"id":2,"method":"quit","params":{}}' \
  | QT_QPA_PLATFORM=offscreen ./bin/toonz_headless
```

Expected responses include `"pong"` and `"bye"`.

## Documentation

- [HEADLESS_API.md](./HEADLESS_API.md) — Full API reference for the headless interface
- [character-design-workflow.md](./character-design-workflow.md) — Character design pipeline extracted from the OpenToonz source code
- [Original build guides](./doc/) — Full OpenToonz build docs for all platforms

## About OpenToonz

OpenToonz is a 2D animation software published by [DWANGO](http://dwango.co.jp/english/). It is based on Toonz Studio Ghibli Version, originally developed by [Digital Video, Inc.](http://www.toonz.com/) and customized by [Studio Ghibli](http://www.ghibli.jp/) over many years of production.

## Licensing

- Files outside of `thirdparty` and `stuff/library/mypaint brushes` are under the [Modified BSD License](./LICENSE.txt).
- For `thirdparty` files, consult the licenses in the respective directories.
- For `stuff/library/mypaint brushes`, see `stuff/library/mypaint brushes/Licenses.txt`.
