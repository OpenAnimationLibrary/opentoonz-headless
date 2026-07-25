# Palette File I/O

The headless `Palette` binding supports native OpenToonz `.tpl` palette files.

## Save a palette

```javascript
var pal = new Palette();
pal.addPage("Landscape");

var sky = pal.addColor(132, 190, 224, 255);
var grass = pal.addColor(92, 143, 69, 255);
var earth = pal.addColor(133, 92, 57, 255);

pal.save("/tmp/countryside.tpl");
```

`save()` requires a path ending in `.tpl`. It writes the same native palette structure used by OpenToonz Studio Palette files.

## Load a palette

```javascript
var pal = new Palette();
pal.load("/tmp/countryside.tpl");

print("styles=" + pal.styleCount);
var color = pal.getStyleColor(2);
print(color.r + "," + color.g + "," + color.b + "," + color.a);
```

`load()` replaces the current contents of the `Palette` object with the styles, pages, names, and other serialized data from the `.tpl` file.

## Round-trip test

After building `toonz_headless`, run:

```bash
QT_QPA_PLATFORM=offscreen \
python3 toonz/sources/toonz_headless/tests/palette_io_test.py
```

The test creates a palette, saves it to `/tmp/opentoonz_headless_palette_io.tpl`, reloads it into a new `Palette`, and verifies the style count and RGBA values.
