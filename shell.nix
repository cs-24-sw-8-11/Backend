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
    

in pkgs.mkShell {
    packages = with pkgs; [
        cmake
        gcc
        compile
        run
        test
        sqlite
    ];
}
