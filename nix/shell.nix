{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # Build tools
    cmake
    gcc
    gnumake
    pkg-config

    # Required libraries
    libnl # For netlink functionality
    
    # Development tools
    gdb
    valgrind
    cppcheck
    libxslt
  ];


} 