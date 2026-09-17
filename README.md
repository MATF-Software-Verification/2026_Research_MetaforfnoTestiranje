# 2026_Research_MetaforfnoTestiranje

## Build requirements

| Tool | Version |
|---|---|
| CMake | 4.2 or newer |
| Conan | 2.0 or newer |
| Ninja | any recent |
| Clang | 22 (tested) |

## Build

Install dependencies:

```sh
conan install . -pr:a profiles/linux-clang-debug --build=missing
```

Build:

```sh
cmake --preset conan-debug
cmake --build --preset conan-debug
```

## VS Code setup

Install the CMake Tools and C/C++ extensions. Then:

1. Run `conan install` as shown above. This creates the presets.
2. Open the project folder.
3. Pick the `conan-debug` preset when VS Code asks.
4. Press F7 to build.