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
        chk-project = pkgs.writeScriptBin "chk-project" ''
            ${pkgs.cpplint}/bin/cpplint --filter='-legal/copyright' --recursive include src

        '';
        ncc = let
            py = pkgs.python311.withPackages (py: with py; [libclang pyyaml]);
        in pkgs.stdenv.mkDerivation {
            pname = "ncc";
            name = "ncc";
            src = pkgs.fetchFromGitHub {
                owner = "nithinn";
                repo = "ncc";
                rev = "master";
                sha256 = "sha256-WL7rMJezxy4Vuvx5F7NavftIPAV19q8qStEIXLWrkno=";
            };
            installPhase = ''
                mkdir -p $out/{share/ncc,bin}
                cp -r $src/* $out/share/ncc
                cat > $out/bin/ncc << EOF
                    ${py.interpreter} $out/share/ncc/ncc.py \$@
                    rm log.txt
                EOF
                chmod +x $out/bin/ncc
            '';
        };

        backend = pkgs.stdenv.mkDerivation {
            name = "Backend";
            pname = "Backend";
            src = pkgs.fetchgit {
                rev = "5e8face";
                url = "https://github.com/cs-24-sw-8-11/Backend";
                hash = "sha256-/ta8ZseNghLOOKIa4oX0QyxWK/NcNc7s7alNRkddtDI=";
                fetchSubmodules = true;
            };
            installPhase = ''
                mkdir -p $out/bin
                cp bin/backend $out/bin/backend
            '';
            nativeBuildInputs = [ pkgs.cmake pkgs.gcc ];
        };

    in {
        devShells.${system}.default = pkgs.mkShell {
            packages = [
                compile
                run
                test
                chk-project
                ncc
                backend
                pkgs.sqlite
                pkgs.sqlite-interactive
                pkgs.cmake
            ];
        };
        packages.${system} = {
            default = backend;
            compile = compile;
            run = run;
            test = test;
            ncc = ncc;
        };
    };
}
