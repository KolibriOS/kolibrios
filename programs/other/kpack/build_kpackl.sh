#!/bin/bash
# Build the native Linux port of kpack (windowless, in-place compressor).
# Self-contained ELF32 - only FASM is needed, no gcc / no linker.
fasm kpackl.asm kpackl
chmod +x kpackl
