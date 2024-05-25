{pkgs, config, lib, ...}: let
    cfg = config.skademaskinen;
in {
    # define some configuration options, also to import into nginx
    options.skademaskinen.p8 = {
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
            default = "${cfg.storage}/p8/prod/Backend";
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
                default = "${cfg.storage}/p8/test/Backend";
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
        systemd.services.p8-prod = if cfg.p8.enable then {
            enable = cfg.p8.enable;
            # to run the sentiment analysis, the service needs access to bash and the python environment
            path = with pkgs; [bash env];
            serviceConfig = {
                WorkingDirectory = cfg.p8.path;
                User = "root";
                ExecStart = "${pkgs.bash}/bin/bash -c '${cfg.p8.path}/build/bin/backend -p ${builtins.toString cfg.p8.port}' -L ${cfg.storage}/p8/prod.log";
                # in case of a failure (e.g. a segfault, restart the server)
                Restart = "on-failure";
            };
            wantedBy = ["default.target"];
            after = ["network-online.target"];
            wants = ["network-online.target"];
        } else {};
        # same setup as the production server, only difference is that it is run with max verbosity
        systemd.services.p8-test = if cfg.p8.test.enable then {
            enable = cfg.p8.test.enable;
            path = with pkgs; [bash env];
            serviceConfig = {
                WorkingDirectory = cfg.p8.test.path;
                User = "root";
                ExecStart = "${pkgs.bash}/bin/bash -c '${cfg.p8.test.path}/build/bin/backend -vvvvvvv -p ${builtins.toString cfg.p8.test.port}'";
                Restart = "on-failure";
            };
            wantedBy = ["default.target"];
            after = ["network-online.target"];
            wants = ["network-online.target"];
        } else {};

        # Updating timers
        # These timers update the system once a day by calling a oneshot service pointing to a script generated at build
        systemd.timers.p8-prod-update = if cfg.p8.enable then {
            enable = cfg.p8.enable;
            wantedBy = ["timers.target"];
            timerConfig = {
                OnBootSec = "5m";
                OnUnitActiveSec = "1d";
                Unit = "p8-prod-update.service";
            };
        } else {};
        systemd.services.p8-prod-update = if cfg.p8.enable then {
            enable = cfg.p8.enable;
            script = ''
                cd ${cfg.p8.path}
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

        systemd.timers.p8-test-update = if cfg.p8.test.enable then {
            enable = cfg.p8.test.enable;
            wantedBy = ["timers.target"];
            timerConfig = {
                OnBootSec = "5m";
                OnUnitActiveSec = "1d";
                Unit = "p8-test-update.service";
            };
        } else {};

        systemd.services.p8-test-update = if cfg.p8.test.enable then {
            enable = cfg.p8.test.enable;
            script = ''
                cd ${cfg.p8.test.path}
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