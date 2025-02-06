{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # Build tools
    stdenv
    autoconf
    automake
    libtool
    pkg-config
    gnumake

    # Dependencies
    libnl
    gcc
  ];

  shellHook = ''
    export PKG_CONFIG_PATH="${pkgs.libnl.dev}/lib/pkgconfig:$PKG_CONFIG_PATH"
    export CPATH="${pkgs.libnl.dev}/include:$CPATH"
    export LIBRARY_PATH="${pkgs.libnl.out}/lib:$LIBRARY_PATH"
  '';
} 