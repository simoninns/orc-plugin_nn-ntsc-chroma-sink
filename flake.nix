{
  description = "NN NTSC Chroma Sink — neural-network NTSC chroma decoder plugin for Decode-Orc";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    # nixpkgs-unstable is required for ONNX Runtime >= 1.23.2.
    # nixos-25.11 ships 1.22.2 which is insufficient for chroma_net_v2.onnx.
    nixpkgs-unstable.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, nixpkgs-unstable, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        pkgs-unstable = import nixpkgs-unstable { inherit system; };
      in {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            cmake
            ninja
            pkg-config
            fftw
            fmt
            git
            ccache
            clang-tools
          ] ++ [
            # ONNX Runtime >= 1.23.2 required; sourced from nixpkgs-unstable.
            pkgs-unstable.onnxruntime
            pkgs-unstable.onnxruntime.dev
          ];

          shellHook = ''
            echo "orc-plugin_nn-ntsc-chroma-sink nix development environment"
            export CMAKE_EXPORT_COMPILE_COMMANDS=1
          '';
        };
      }
    );
}
