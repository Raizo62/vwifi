{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # Build tools
    cmake
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