{ pkgs ? import <nixpkgs> { } }:
let
  odin-stx = pkgs.llvmPackages_17.stdenv.mkDerivation (rec {
    name = "stx";
    src = ./.;
    dontConfigure = true;
    nativeBuildInputs = [ pkgs.git ];
  });
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [     
    bintools
    llvm
    clang_18
    libclang
  ];
}