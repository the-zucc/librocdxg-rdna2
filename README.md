# librocdxg RDNA2 Patch

An experimental fork of
[ROCm/librocdxg](https://github.com/ROCm/librocdxg).

This repository adapts librocdxg for use with selected RDNA2 systems under
WSL2 and includes entries for additional RDNA2 PCI IDs.

This work was inspired by a fork of an earlier librocdxg commit:
[joshEng1/librocdxg-gfx1032fix](https://github.com/joshEng1/librocdxg-gfx1032fix).

It is not a standalone runtime. It builds and installs librocdxg with these
extra compatibility changes while relying on the normal ROCm stack, Windows
driver, and WSL environment.

## Why this fork exists

RDNA2 is not included in upstream librocdxg's published WSL compatibility
matrix. Some RDNA2 configurations can work with additional device information,
gfx-version override handling, and queue-related changes.

This repository is mainly intended for experimentation with the listed devices
and for the Ollama setup described below.

## RDNA2 coverage

The fork currently contains entries for:

| Family | Native ISA | PCI IDs | GPU models associated with these IDs |
| --- | --- | --- | --- |
| Navi 21 | gfx1030 | `73BF`, `73AF`, `73A5` | RX 6800, RX 6800 XT, RX 6900 XT, RX 6950 XT |
| Navi 22 | gfx1031 | `73DF` | RX 6700, RX 6700 XT, RX 6750 XT, RX 6800M, RX 6850M XT |
| Navi 23 | gfx1032 | `73E3`, `73EF`, `73FF` | Radeon PRO W6600, RX 6600, RX 6600 XT, RX 6600M, RX 6650 XT, RX 6700S, RX 6800S |
| Navi 24 | gfx1034 | `743F`, `7424` | RX 6300, RX 6400, RX 6500 XT, RX 6500M |

Several products share PCI IDs, so an entry does not guarantee identical
behavior across every card using that ID. The table describes what the fork
recognizes; it is not an official support list.

## Install

First install ROCm and its normal WSL prerequisites using AMD's
[ROCm installation guide](https://rocm.docs.amd.com/projects/install-on-linux/en/latest/install/quick-start.html).
You also need WSL2, a compatible AMD Windows driver, CMake, GCC, and the Windows
SDK.

Clone and build this fork inside WSL:

```bash
git clone https://github.com/the-zucc/librocdxg-rdna2.git
cd librocdxg-rdna2

# Adjust the SDK version if necessary.
export win_sdk='/mnt/c/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0'

mkdir -p build
cd build
cmake .. -DWIN_SDK="${win_sdk}/shared"
make -j"$(nproc)"
sudo make install
```

The fork installs librocdxg into the existing ROCm prefix, replacing the
installed librocdxg library while continuing to use the rest of the upstream
ROCm stack.

## Enable and verify RDNA2

Set the environment used for the tested RDNA2 setup. The gfx override makes the
device appear as gfx1030 to the ROCm runtime:

```bash
export HSA_ENABLE_DXG_DETECTION=1
export HSA_OVERRIDE_GFX_VERSION=10.3.0
export HSA_ENABLE_SDMA=0
export HSA_ENABLE_PEER_SDMA=0
export LIBROCDXG_ALLOC_USER_QUEUE_FROM_UMD=1

rocminfo
```

A working adapter should appear as a GPU agent. For example, the Navi 22 test
system used for this fork reports:

```text
Name:                    gfx1030
Marketing Name:          AMD Radeon RX 6700 XT
Chip ID:                 29663(0x73df)
Compute Unit:            40
```

These settings are included because they were used with the working setup.
Other combinations may also work, but have not been validated here.

## Ollama

Create an Ollama service override with `sudo systemctl edit ollama`:

```systemd
[Service]
Environment="HSA_ENABLE_DXG_DETECTION=1"
Environment="HSA_OVERRIDE_GFX_VERSION=10.3.0"
Environment="HSA_ENABLE_SDMA=0"
Environment="HSA_ENABLE_PEER_SDMA=0"
Environment="LIBROCDXG_ALLOC_USER_QUEUE_FROM_UMD=1"
Environment="OLLAMA_FLASH_ATTENTION=1"
```

Apply it with:

```bash
sudo systemctl daemon-reload
sudo systemctl restart ollama
```

## Project scope

The goal is to keep this RDNA2 experiment usable as upstream librocdxg evolves.
For general librocdxg usage, releases, and official compatibility information,
see the
[upstream repository](https://github.com/ROCm/librocdxg) for those details.

Behavior can vary with the exact GPU, Windows driver, ROCm release, and code
objects shipped by an application. The RX 6700 XT configuration shown above was
verified with `rocminfo`; other entries have not all been tested on hardware in
this repository.
