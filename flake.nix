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
            ${build-dir}/bin/backend $@
        '';
        test = pkgs.writeScriptBin "test-project" ''
            ${pkgs.cmake}/bin/ctest --test-dir ${build-dir} $@
        '';
        fmt-project = pkgs.writeScriptBin "fmt-project" ''
            ${pkgs.cpplint}/bin/cpplint --filter='-legal/copyright,-whitespace/line_length,-build/namespaces' --recursive include src

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
                    result=\$(${py.interpreter} $out/share/ncc/ncc.py \$@)
                    echo \$result
                    rm log.txt
                    if [[ -z \$result ]]; then
                        exit 0
                    fi
                    exit 1
                EOF
                chmod +x $out/bin/ncc
            '';
        };

        backend = pkgs.stdenv.mkDerivation {
            name = "P8 Backend";
            src = pkgs.fetchgit {
                url = "https://github.com/cs-24-sw-8-11/Backend";
                sha256 = "sha256-Qxe9NYlMnG/C5So+wlwyKz7rYj5ot6ieKAcd932DdMw=";
                fetchSubmodules = true;
            };
            buildInputs = with pkgs; [cmake];
            installPhase = ''
                mkdir -p $out
                cp -r bin $out
                ln -s $out/bin/backend $out/bin/P8-Backend
            '';
        };

    in {
        devShells.${system}.default = pkgs.mkShell {
            packages = [
                pkgs.gcc
                compile
                run
                test
                fmt-project
                ncc
                pkgs.sqlite
                pkgs.sqlite-interactive
                pkgs.cmake
                pkgs.jq
                pkgs.python311Packages.transformers
                pkgs.python311Packages.pytorch
                pkgs.cppcheck
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
