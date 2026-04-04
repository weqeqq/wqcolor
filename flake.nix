{
  description = "wqcolor: C++23 color conversion library for byte pixel buffers";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    wqparallel = {
      url = "github:weqeqq/wqparallel";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { nixpkgs, wqparallel, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forAllSystems = f: nixpkgs.lib.genAttrs systems f;

      perSystem =
        system:
        let
          pkgs = import nixpkgs { inherit system; };
          lib = pkgs.lib;
          gtestPkg = if pkgs ? gtest then pkgs.gtest else pkgs.googletest;

          src = lib.fileset.toSource {
            root = ./.;
            fileset = lib.fileset.unions [
              ./README.md
              ./headers
              ./meson.build
              ./meson_options.txt
              ./sources
              ./subprojects/.wraplock
              ./subprojects/gtest.wrap
              ./subprojects/highway.wrap
              ./subprojects/wqparallel.wrap
              ./tests
            ];
          };

          mkWqcolor =
            {
              tests ? false,
            }:
            pkgs.stdenv.mkDerivation {
              pname = "wqcolor";
              version = "0.1.1";
              inherit src;

              strictDeps = true;

              nativeBuildInputs =
                with pkgs;
                [
                  meson
                  ninja
                  pkg-config
                ];

              buildInputs =
                [
                  wqparallel.packages.${system}.default
                  pkgs.libhwy
                ]
                ++ lib.optionals tests [ gtestPkg ];

              mesonFlags = [
                "-Dinstall=enabled"
                "-Dsimd=enabled"
                "-Dtests=${if tests then "enabled" else "disabled"}"
              ];

              doCheck = tests;

              meta =
                with lib;
                {
                  description = "Deterministic C++23 color conversion library for byte pixel buffers";
                  platforms = platforms.unix;
                };
            };

          package = mkWqcolor { };
          checkPackage = mkWqcolor { tests = true; };
        in
        {
          inherit pkgs package checkPackage;
        };
    in
    {
      packages = forAllSystems (
        system:
        let
          inherit (perSystem system) package;
        in
        {
          default = package;
          wqcolor = package;
        }
      );

      checks = forAllSystems (
        system:
        let
          inherit (perSystem system) checkPackage;
        in
        {
          default = checkPackage;
          wqcolor-tests = checkPackage;
        }
      );

      devShells = forAllSystems (
        system:
        let
          inherit (perSystem system) checkPackage pkgs;
        in
        {
          default = pkgs.mkShell {
            inputsFrom = [ checkPackage ];

            packages = with pkgs; [
              clang-tools
              cmake
              doxygen
              nixpkgs-fmt
              python3
            ];
          };
        }
      );

      formatter = forAllSystems (system: (perSystem system).pkgs.nixpkgs-fmt);
    };
}
