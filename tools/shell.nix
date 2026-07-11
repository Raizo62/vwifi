{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  # Build tools
  nativeBuildInputs = with pkgs; [
    cmake
    pkg-config
  ];

  buildInputs = with pkgs; [
    # Required libraries
    libnl # For netlink functionality

    # Development tools
    gdb
    valgrind
    cppcheck
    libxslt
  ];
}
