{
  description = "Emulator for the XJ24 microprocessor";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      rec {
        defaultPackage = pkgs.stdenv.mkDerivation {
          pname = "xjx";
          version = "1.2";
          src = ./.;

          nativeBuildInputs = with pkgs; [
            ncurses
            pkg-config
          ];
        };

        devShell = pkgs.mkShell {
          packages = with pkgs; [
            bashInteractive
            bear
          ] ++ defaultPackage.nativeBuildInputs;
        };
      });
}
