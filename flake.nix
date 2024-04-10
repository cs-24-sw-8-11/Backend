{
    description = "P8 Project nix flake";
    inputs = {
        nixpkgs.url = "nixpkgs/nixos-unstable";
    };
    outputs = { self, nixpkgs }: let 
        system = "x86_64-linux";
        pkgs = nixpkgs.legacyPackages.${system};
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

        backend = pkgs.stdenv.mkDerivation {
            name = "Backend";
            pname = "Backend";
            src = pkgs.fetchgit {
                rev = "6fe8a6c";
                url = "https://github.com/cs-24-sw-8-11/Backend";
                hash = "sha256-lxewBXvm1jCP6lYFtGL57CIknYvEXD+8N0Ktfk4Mgcc=";
                fetchSubmodules = true;
            };
            nativeBuildInputs = [ pkgs.cmake pkgs.gcc ];
        };

    in {
        devShells.${system}.default = pkgs.mkShell {
            packages = [
                compile
                run
                test
                backend
                pkgs.sqlite
                pkgs.sqlite-interactive
                pkgs.cmake
            ];
        };
        packages.default = backend;
    };
}
