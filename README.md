# trj-render

**trj-render** is a lightweight C++17 tool that reads LAMMPS trajectory files (`.lammpstrj`) and renders each frame as a 2D PNG image. It projects 3D atomic coordinates onto a customizable viewing plane, allowing rotation, scaling, and selective frame rendering.

## Features

- Parse and visualize `.lammpstrj` trajectory files frame by frame  
- Adjustable **rotation angles** around X, Y, and Z axes  
- Configurable **scaling factor** (or automatic adjustment)  
- Selective rendering of a **specific frame**  
- Renders both **simulation box edges** and **atoms** with per-type colors and radii  
- Output image format: **PNG** (via [lodepng](https://github.com/lvandeve/lodepng))

## Dependencies

All dependencies are header-only libraries included via `external/`:

| Library | Purpose | License |
|----------|----------|----------|
| [lammpstrj-parser](https://github.com/wtnb-appi/lammpstrj-parser) | LAMMPS trajectory reader | MIT |
| [lodepng](https://github.com/lvandeve/lodepng) | PNG encoding | Zlib |
| [cxxopts](https://github.com/jarro2783/cxxopts) | Command-line parser | MIT |

## Build

### Requirements
- C++17-compatible compiler (tested with GCC ≥ 8.5.0)
- GNU Make

```sh
make
```

## Usage

```bash
./trj2png [options] filename
```

### Options

| Option | Description |
|---------|--------------|
| `-x, --rx <deg>` | Rotation around **X-axis** (degrees) |
| `-y, --ry <deg>` | Rotation around **Y-axis** (degrees) |
| `-z, --rz <deg>` | Rotation around **Z-axis** (degrees) |
| `-s, --scale <num>` | Scale factor for the simulation box → pixels (if negative, the scale is automatically adjusted so that the larger side of the image becomes 800 px) |
| `-f, --frame <idx>` | Render only the specified frame (0-based). If omitted, all frames are rendered. |
| `--radiusN <num>` | Radius of atom type **N** (0–15). Only applied if specified. |
| `-h, --help` | Show help message |

## Examples

Render all frames, rotating by 30° around Y and −15° around Z, scaling by 200:
```bash
./trj2png -y 30 -z -15 -s 200 sample.lammpstrj
```

Render only frame 3, rotating by 45° around X, with automatic scaling:
```bash
./trj2png --rx 45 --scale -1 --frame 3 sample.lammpstrj
```

## Output

- Each frame is saved as a PNG file named:
  ```
  frame.0000.png
  frame.0001.png
  ...
  ```
- Simulation box edges are drawn as black lines.
- Atom color, border, and radius are automatically assigned based on atom type.

## License

MIT License © 2025 Hiroshi Watanabe  
See individual library directories for their respective licenses.
