# orc-plugin_nn-ntsc-chroma-sink

External Decode-Orc stage plugin for neural-network NTSC chroma decoding.

This repository follows the same external-plugin patterns as the canonical skeleton while implementing the `nn_ntsc_chroma_sink` stage.

## What It Contains

- Stage implementation: `nn_ntsc_chroma_sink`
- Required plugin entrypoints:
      - `orc_get_stage_plugin_descriptor`
      - `orc_register_stage_plugin`
- Local smoke tests for stage metadata and entrypoint registration
- Cross-platform CI workflow targets for Linux, macOS, and Windows
- Release workflow that uploads platform plugin artifacts

## SDK Contract (Current)

- Include only public plugin SDK headers in plugin/stage code:

```cpp
#include <orc/plugin/orc_plugin_sdk.h>
```

- Export the two required entrypoints:
      - `orc_get_stage_plugin_descriptor`
      - `orc_register_stage_plugin`

- Set descriptor versions from SDK constants:
      - `orc::kStagePluginHostAbiVersion`
      - `orc::kStagePluginApiVersion`

## Plugin Version Source

- The plugin descriptor version is set at CMake configure time.
- Default value is `project(VERSION ...)` from `CMakeLists.txt`.
- In GitHub Actions tag builds, tags like `v1.2.3` are automatically mapped to plugin version `1.2.3`.
- You can always override manually with:

```bash
cmake -S . -B build -DORC_PLUGIN_VERSION=1.2.3
```

## Requirements

- ONNX Runtime >= 1.23.2
- FFTW3
- [Decode-Orc plugin SDK](https://github.com/simoninns/decode-orc)

## Local Build (installed SDK)

1. Install decode-orc (or at minimum its plugin SDK package) to a prefix.
2. Configure and build this plugin against that install prefix:

```bash
cmake -S . -B build \
      -DCMAKE_PREFIX_PATH=/absolute/path/to/decode-orc-install
cmake --build build --parallel
```

3. Run tests:

```bash
ctest --test-dir build --output-on-failure -C Release
```

## Local Build (against in-tree decode-orc SDK)

1. Clone decode-orc and this repository.
2. Enter the Nix development shell:

```bash
nix develop
```

3. Configure and build against the in-tree SDK checkout:

```bash
cmake -S . -B build \
      -DORC_INTREE_SDK_DIR=/absolute/path/to/decode-orc \
      -DBUILD_TESTS=ON
cmake --build build --parallel
```

4. Run tests:

```bash
ctest --test-dir build --output-on-failure
```

5. Package local artifact:

```bash
./scripts/package_local.sh build dist
```

The packaged plugin is written to `dist/orc-plugin_nn-ntsc-chroma-sink_<platform>.<ext>` and can be copied into Decode-Orc's local plugin cache for smoke testing.

## Artifact Naming Contract

Release assets and local packaging output follow:

- `orc-plugin_nn-ntsc-chroma-sink_linux.so`
- `orc-plugin_nn-ntsc-chroma-sink_macos.dylib`
- `orc-plugin_nn-ntsc-chroma-sink_windows.dll`

## CI and Release

- Unified workflow: `.github/workflows/ci.yml`
      - on push/PR: builds/tests/packages on Linux and macOS via Nix (`nix develop`), plus native Windows build; uploads CI artifacts
      - on tag push (`v*`): runs the exact same build/test/package matrix, then publishes those artifacts to GitHub Release assets

## Tagging a Release

First, check existing tags to avoid conflicts:

```bash
git tag --list
```

Then, tag a commit with a version matching the pattern `v*` (for example `v1.0.0`):

```bash
git tag v1.0.0
git push origin v1.0.0
```

This triggers the release workflow, which:
1. Builds and tests across Linux, macOS, and Windows
2. Packages platform-specific artifacts
3. Creates a GitHub Release with the tagged artifacts attached

## License

GPL-3.0-or-later

