{ pkgs ? import <nixpkgs> {} }:

let
  revisionId = pkgs.writeShellScriptBin "get-current-cmtid" ''
    git -C "''${ROOT:-.}" rev-parse --short HEAD
  '';

  clink = pkgs.stdenvNoCC.mkDerivation {
    pname = "clink";
    version = "unstable";
    src = ./programs/develop/clink;

    nativeBuildInputs = [ pkgs.gcc ];
    buildPhase = ''
      gcc main.c -o clink
    '';
    installPhase = ''
      install -Dm755 clink "$out/bin/clink"
    '';
  };

  objconv = pkgs.stdenvNoCC.mkDerivation {
    pname = "objconv";
    version = "unstable";
    src = ./programs/develop/objconv;

    nativeBuildInputs = [ pkgs.gcc ];
    buildPhase = ''
      g++ -O2 -o objconv *.cpp
    '';
    installPhase = ''
      install -Dm755 objconv "$out/bin/objconv"
    '';
  };

  kos32Tcc = pkgs.stdenvNoCC.mkDerivation {
    pname = "kos32-tcc";
    version = "unstable";
    src = ./programs/develop/ktcc/bin/kos32-tcc;
    dontUnpack = true;

    installPhase = ''
      install -Dm755 "$src" "$out/bin/kos32-tcc"
    '';
  };

  kpack = pkgs.stdenvNoCC.mkDerivation {
    pname = "kpack";
    version = "0.11";
    src = ./programs/other/kpack/kerpack_linux;

    nativeBuildInputs = with pkgs; [ fasm gnumake gcc ];

    installPhase = ''
      install -Dm755 kpack "$out/bin/kpack"
      install -Dm755 kerpack "$out/bin/kerpack"
    '';
  };

  cmm = pkgs.stdenvNoCC.mkDerivation {
    pname = "c--";
    version = "0.239";
    src = ./programs/develop/cmm;

    hardeningDisable = [ "format" "fortify" "stackprotector" ];
    nativeBuildInputs = [ pkgs.gnumake pkgs.pkgsi686Linux.stdenv.cc ];
    buildInputs = [ pkgs.pkgsi686Linux.glibc.static ];
    makeFlags = [
      "-f"
      "Makefile.lin32"
      "CC=${pkgs.pkgsi686Linux.stdenv.cc}/bin/cc"
    ];

    installPhase = ''
      install -Dm755 c-- "$out/bin/c--"
      install -Dm644 c--.ini "$out/bin/c--.ini"
    '';
  };

  kolibrios-toolchain = pkgs.stdenvNoCC.mkDerivation {
    pname = "kolibrios-toolchain";
    version = "5.4.0";

    src = pkgs.fetchurl {
      url = "http://ftp.kolibrios.org/users/Serge/new/Toolchain/x86_64-linux-kos32-5.4.0.7z";
      sha256 = "7ded2eafac38362fd51bcf15b721657edf57e3f9789bf3adb1387094128c92c6";
    };
    isl10 = pkgs.fetchurl {
      url = "http://board.kolibrios.org/download/file.php?id=8301libisl.so.10.2.2.7z";
      sha256 = "404423f396b4fda08daf3687fd44ef5e006fafc0cfc81fbc1ffd5f5c9b337761";
    };

    dontUnpack = true;
    nativeBuildInputs = [ pkgs.p7zip pkgs.autoPatchelfHook ];
    buildInputs = with pkgs; [ stdenv.cc.cc.lib gmp mpfr libmpc ];

    installPhase = ''
      7z x -y -o"$out" "$src"
      mkdir -p "$out/lib"
      7z x -y -o"$out/lib" "$isl10"
      ln -s libisl.so.10.2.2 "$out/lib/libisl.so.10"
      ln -s ${pkgs.mpfr}/lib/libmpfr.so.6 "$out/lib/libmpfr.so.4"
    '';

    preFixup = ''
      addAutoPatchelfSearchPath "$out/lib"
    '';
  };

  jwasm211 = pkgs.stdenvNoCC.mkDerivation {
    pname = "jwasm";
    version = "2.11";
    src = pkgs.fetchurl {
      url = "https://sourceforge.net/projects/jwasm/files/JWasm%20Linux%20binary/JWasm211bl.zip/download";
      sha256 = "20fe6448f7a6b00d23e5484761e729fba59170ec09d8da8598eaa9081e0c53c4";
    };
    nativeBuildInputs = [ pkgs.unzip ];
    dontUnpack = true;
    installPhase = ''
      mkdir -p "$out/bin"
      ${pkgs.unzip}/bin/unzip -p "$src" jwasm > "$out/bin/jwasm"
      chmod +x "$out/bin/jwasm"
    '';
  };

  nativePackages = with pkgs; [
    tup
    fasm
    nasm
    gnumake
    gcc
    binutils
    python3
    mtools
    cdrkit
    parted
    gptfdisk
    p7zip
    zip
    unzip
    wget
    curl
    file
    diffutils
    gawk
    gnused
    findutils
    patchelf
    gmp
    mpfr
    libmpc
    isl
    zlib
    cl
  ] ++ [ clink cmm jwasm211 kolibrios-toolchain kpack kos32Tcc objconv revisionId ];
in
pkgs.mkShell {
  packages = nativePackages;

  shellHook = ''
    export ROOT="$PWD"
    export KOS32_TOOLCHAIN="''${KOS32_TOOLCHAIN:-${kolibrios-toolchain}}"

    if [ ! -d "$KOS32_TOOLCHAIN/win32/bin" ] && [ -d /home/autobuild/tools/win32/bin ]; then
      export KOS32_TOOLCHAIN=/home/autobuild/tools
    fi

    if [ -d "$KOS32_TOOLCHAIN/win32/bin" ]; then
      export PATH="${pkgs.fasm}/bin:$KOS32_TOOLCHAIN/win32/bin:$PATH"
      mkdir -p "$TMPDIR/kolibri-toolchain-bin"
      ln -sf "$KOS32_TOOLCHAIN/win32/bin/kos32-strip" "$TMPDIR/kolibri-toolchain-bin/strip"
      export PATH="$TMPDIR/kolibri-toolchain-bin:$PATH"
      if [ -r "$KOS32_TOOLCHAIN/win32/bin/kos32-export-env-vars" ]; then
        source "$KOS32_TOOLCHAIN/win32/bin/kos32-export-env-vars" "$ROOT"
      fi
    else
      echo "KGCC not found. Set KOS32_TOOLCHAIN to the installed tools directory."
      echo "Expected: $KOS32_TOOLCHAIN/win32/bin/kos32-gcc"
    fi

    if [ -n "''${JWASM:-}" ] && [ -x "$JWASM" ]; then
      mkdir -p "$TMPDIR/kolibri-jwasm-bin"
      ln -sf "$JWASM" "$TMPDIR/kolibri-jwasm-bin/jwasm"
      export PATH="$TMPDIR/kolibri-jwasm-bin:$PATH"
    fi

    echo "KolibriOS build shell ready (ROOT=$ROOT)"
  '';
}
