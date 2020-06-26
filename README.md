<h1 align="center">
  <br>
  <img src="https://raw.githubusercontent.com/eclmist/ether/master/docs/ether.png" alt="Ether-Logo" width="200"></a>
  <br>
  Ether
  <br>
</h1>
<h4 align="center">A simple and lightweight DX12 renderer.</h4>

## Introduction

Ether is a 3D renderer written with DirectX 12 and C++, meant to be used as a platform for creating and testing modern graphics rendering techniques. 

Ether is open-sourced, licensed under the [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.en.html).

## Requirements

As Ether is a DirectX 12 renderer, it can only be built and ran on a Windows 10 machine.

Furthermore, ensure that your hardware supports:

    - DirectX 12
    - Shader Model 5.0

## Getting Started

> Note: Ether is currently a WIP and is considered unstable. Proceed with caution at your own risk.

Ether uses CMake to build it's binaries. If you would like to build Ether yourselves, run the following CMake commands to generate project files and compile with your favourite C++ compiler.
All targets (debug, release, library) will be generated automatically.

```shell
$ cmake -B build -S .
```

```
$ cmake --build ./build/
```

This will generate OS specific project in the `/build/` folder, and will build the Ether binaries into `/bin/`.