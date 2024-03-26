{pkgs ? import <nixpkgs> {}, ...}: let
    pkg = pkgs.writeScriptBin "run-backend" ''
        cmake -B build
        cmake --build build
        ./build/backend
    '';
    
    crow = pkgs.stdenv.mkDerivation rec {
        name = "Crow";
        pname = name;
        src = pkgs.fetchFromGitHub {
            owner = "CrowCpp";
            repo = name;
            rev = "ad337a8a868d1d6bc61e9803fc82f36c61f706de";
            sha256 = "sha256-3bRYz4pkzIKwYFGb/n9twV/01O8LWIdlXMHHyaRju74=";
        };
        nativeBuildInputs = [pkgs.cmake pkgs.asio];

    };

in pkgs.mkShell {
    packages = with pkgs; [
        cmake
        gcc
        pkg
        sqlitecpp
        sqlite
        crow
        asio
    ];
}
