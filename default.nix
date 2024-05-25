# ----- DISCLAIMER -----
# The project group is aware that this module is not reproducible, but its needed to document the test setup
# ----------------------
# The tests were set up by cloning the project to `cfg.path` and `cfg.test.path`, compiling it (using the `build-project` command defined in flake.nix),
# and populating the database by running `./build/bin/backend populate`
 
{pkgs, config, lib, ...}: let
    cfg = config.p8;
in {
    # define some configuration options, also to import into nginx
    options.p8 = {
        # Enable the services
        enable = lib.mkOption {
            type = lib.types.bool;
            default = false;
        };
        # Port to listen on
        port = lib.mkOption {
            type = lib.types.int;
            default = 8800;
        };
        # Path to the compiled project
        path = lib.mkOption {
            type = lib.types.str;
        };
        # options specific to the test server.
        test = {
            enable = lib.mkOption {
                type = lib.types.bool;
                default = false;
            };
            port = lib.mkOption {
                type = lib.types.int;
                default = 8801;
            };
            path = lib.mkOption {
                type = lib.types.str;
            };
        };
    };

    config = let
        # python environment to use for sentiment analysis
        env = pkgs.python311.withPackages (py: with py; [
            transformers
            pytorch
        ]);
    in {
        # Systemd service for the production server
        systemd.services.p8-prod = if cfg.enable then {
            enable = cfg.enable;
            # to run the sentiment analysis, the service needs access to bash and the python environment
            path = with pkgs; [bash env];
            serviceConfig = {
                WorkingDirectory = cfg.path;
                User = "root";
                ExecStart = "${pkgs.bash}/bin/bash -c '${cfg.path}/build/bin/backend -p ${builtins.toString cfg.port}' -L ${cfg.path}/prod.log";
                # in case of a failure (e.g. a segfault, restart the server)
                Restart = "on-failure";
            };
            wantedBy = ["default.target"];
            after = ["network-online.target"];
            wants = ["network-online.target"];
        } else {};
        # same setup as the production server, only difference is that it is run with max verbosity
        systemd.services.p8-test = if cfg.test.enable then {
            enable = cfg.test.enable;
            path = with pkgs; [bash env];
            serviceConfig = {
                WorkingDirectory = cfg.test.path;
                User = "root";
                ExecStart = "${pkgs.bash}/bin/bash -c '${cfg.test.path}/build/bin/backend -vvvvvvv -p ${builtins.toString cfg.test.port}'";
                Restart = "on-failure";
            };
            wantedBy = ["default.target"];
            after = ["network-online.target"];
            wants = ["network-online.target"];
        } else {};

        # Updating timers
        # These timers update the system once a day by calling a oneshot service pointing to a script generated at build
        systemd.timers.p8-prod-update = if cfg.enable then {
            enable = cfg.enable;
            wantedBy = ["timers.target"];
            timerConfig = {
                OnBootSec = "5m";
                OnUnitActiveSec = "1d";
                Unit = "p8-prod-update.service";
            };
        } else {};
        systemd.services.p8-prod-update = if cfg.enable then {
            enable = cfg.enable;
            script = ''
                cd ${cfg.path}
                systemctl stop p8-prod.service
                ${pkgs.su}/bin/su mast3r -c '${pkgs.git}/bin/git pull'
                ${pkgs.cmake}/bin/cmake -B build
                ${pkgs.cmake}/bin/cmake --build build
                systemctl start p8-prod.service
            '';
            serviceConfig = {
                Type = "oneshot";
                User = "root";
            };
        } else {};

        systemd.timers.p8-test-update = if cfg.test.enable then {
            enable = cfg.test.enable;
            wantedBy = ["timers.target"];
            timerConfig = {
                OnBootSec = "5m";
                OnUnitActiveSec = "1d";
                Unit = "p8-test-update.service";
            };
        } else {};

        systemd.services.p8-test-update = if cfg.test.enable then {
            enable = cfg.test.enable;
            script = ''
                cd ${cfg.test.path}
                systemctl stop p8-test.service
                ${pkgs.su}/bin/su mast3r -c '${pkgs.git}/bin/git pull'
                ${pkgs.cmake}/bin/cmake -B build
                ${pkgs.cmake}/bin/cmake --build build
                systemctl start p8-test.service
            '';
            serviceConfig = {
                Type = "oneshot";
                User = "root";
            };
        } else {};
    };
}