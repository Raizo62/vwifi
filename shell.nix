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

  shellHook = ''
    echo "vwifi development environment"
    echo "Available tools:"
    echo " - cmake (build system)"
    echo " - gcc (compiler)"
    echo " - gdb (debugger)"
    echo " - valgrind (memory checker)"
    echo " - cppcheck (static analyzer)"
    echo ""
    echo "To build:"
    echo "mkdir -p build && cd build"
    echo "cmake .."
    echo "cmake --build . -j$(nproc)"
  '';
} 