{ pkgs ? import <nixpkgs> { } }:

let
in pkgs.mkShell {
  nativeBuildInputs = [
    pkgs.gcc
    pkgs.cmake
    pkgs.SDL2
    pkgs.glew
    pkgs.glm
    pkgs.nlohmann_json
    pkgs.entr
  ];

  shellHook = ''
    export LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib:$LD_LIBRARY_PATH"
    echo "Nix env activated."
  '';
}
