<h1 align="center">
  <br>
  <img src="https://raw.githubusercontent.com/eclmist/ether/master/docs/ether.png" alt="Ether-Logo" width="200"></a>
  <br>
  Ether
  <br>
</h1>
<h4 align="center">A realtime DirectX12 rendering engine</h4>

<p align="center">
  <a href="https://github.com/Eclmist/Ether/actions">
    <img src="https://github.com/Eclmist/Ether/workflows/Build/badge.svg" alt="Build Status">
  </a>
  <a href="#------">
    <img src="https://img.shields.io/badge/stability-experimental-orange.svg">
  </a>
  <a href="https://www.gnu.org/licenses/gpl-3.0.en.html">
    <img src="https://img.shields.io/badge/license-GPL3--or--later-blue.svg">
  </a>
</p>

## Introduction

Ether is a 3D renderer written with DirectX 12 and C++, meant to be used as a platform for creating and testing modern graphics rendering techniques. 

Ether is open-sourced, licensed under the [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.en.html).

## Features
* Physically Based Rendering - UE4 Microfacet BRDF, Image Based Lighting
* Raytracing with DXR (AO, shadows)
* Bindless Textures
* Post Processing (TAA, Bloom)
* GPU Resource/Descriptor Management
* Shader Hot Reloading 
* Asset Pipeline /w Tool Integration ([Matcha Editor](https://github.com/Eclmist/Matcha-Editor))

## Requirements

As Ether is a DirectX 12 renderer, it can only be built and ran on a Windows 10 machine that supports DirectX 12.

Ether uses [SMath](https://github.com/Eclmist/SMath) as a submodule. As such, it is necessary to pull submodules when cloning Ether.

## Getting Started

> Note: Ether is currently a very early WIP and is considered unstable and not particularly useful. Proceed with caution at your own risk.

Ether uses CMake to build it's binaries. If you would like to build Ether yourselves, make sure that CMake is installed and run the included build script `GenerateProjects.bat`.

All targets (debug, release, library) will be generated automatically.

To compile, simply run

```
$ cmake --build ./build/
```

This will generate projects in the `/build/` folder, and will build the Ether binaries into `/bin/`.

## Screenshots
<p align="center">
  <img src="https://raw.githubusercontent.com/eclmist/ether/develop/docs/suntemple.png" width=1000>
  <img src="https://raw.githubusercontent.com/eclmist/ether/develop/docs/sponza2.png" width=1000>
  <img src="https://raw.githubusercontent.com/eclmist/ether/develop/docs/san-miguel.png" width=1000>
  <img src="https://raw.githubusercontent.com/eclmist/ether/develop/docs/bistro.png" width=1000>
  <img src="https://raw.githubusercontent.com/eclmist/ether/master/docs/brdf.png" width=1000>
</p>

