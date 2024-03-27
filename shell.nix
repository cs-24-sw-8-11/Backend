{pkgs ? import <nixpkgs> {}, ...}: let
    build-dir = "./build";
    compile = pkgs.writeScriptBin "build-project" ''
        ${pkgs.cmake}/bin/cmake -B ${build-dir}
        ${pkgs.cmake}/bin/cmake --build ${build-dir}
    '';
    run = pkgs.writeScriptBin "run-project" ''
        ${build-dir}/src/backend $@
    '';
    test = pkgs.writeScriptBin "test-project" ''
        ${pkgs.cmake}/bin/ctest --test-dir ${build-dir} $@
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
        compile
        run
        test
        sqlitecpp
        sqlite
        crow
        asio
        argparse
        nlohmann_json
    ];
}
