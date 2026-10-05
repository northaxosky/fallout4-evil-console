# Fallout 4 Evil Console

A Dear Modding UI replacement for the Fallout 4 console, built with CommonLibF4 and xmake.

## Set up

Clone with submodules:

```powershell
git clone --recurse-submodules <repository-url> fallout4-evil-console
cd fallout4-evil-console
```

Requires Windows, xmake 3.1.1 or newer, and either MSVC or Clang-CL with the Windows SDK. Verification also requires `clang-format`.

The single `plugin_version` value at the top of `xmake.lua` controls binary metadata, archive names, tags, and releases.

## Build

`releasedbg` is the default distribution mode:

```powershell
xmake build
xmake f -m debug
xmake build
```

Build outputs remain under `build\windows\x64\<mode>`. Every build ensures the selected DLL and PDB are staged in `package\F4SE\Plugins`.

To copy the complete package payload to an isolated development directory:

```powershell
xmake f --deploy_dir="C:\Mods\Evil Console - Dev"
xmake build
```

The destination is the full directory: no plugin-name directory is appended. Existing unrelated files are preserved. Clear the saved destination with:

```powershell
xmake f --deploy_dir=
```

The legacy `FO4_DEV_MODS`, `XSE_FO4_MODS_PATH`, and `XSE_FO4_GAME_PATH` variables are ignored.

## Package and verify

Authored Data-root files belong under `package\`. Generated DLL/PDB files in `package\F4SE\Plugins` are ignored. Nexus text and optional artwork stay outside the payload; place an optional logo at `nexus\logo.png`.

Create the release ZIP from the canonical package contents:

```powershell
xmake f -m releasedbg
xmake pack -f zip
```

The archive is written to `build\xpack\<plugin-name>\`.

Run the same verification entry point used by CI:

```powershell
xmake f --toolchain=msvc --deploy_dir=
xmake verify
```

Use `--toolchain=clang-cl` to check Clang-CL instead. Verification exercises both modes, cached staging, asset-only deployment, environment isolation, and archive contents. It uses temporary destinations, restores your configuration, and never launches Fallout 4.

## Runtime and releases

Runtime support is inherited from the pinned CommonLibF4 revision. The exported OG query/load and newer preload entry points are retained.

On `main`, increasing `plugin_version` creates `v<version>` and immediately publishes the verified `releasedbg` ZIP. Pull requests, feature branches, unchanged versions, and the repository's initial version do not publish. Existing conflicting tags or assets cause the release job to fail instead of overwriting them.

`xmake release-info` reports the version, tag, and archive path from xmake metadata. CI uses `--before=<commit>` to detect a version increase and publishes the same archive it verified.

This project is distributed under [LICENSE](LICENSE) with the additional terms in [EXCEPTIONS](EXCEPTIONS).
