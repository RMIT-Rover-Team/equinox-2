{
  description = "Equinox 2 development environments";

  inputs = {
    # Keep the complete GStreamer stack on one package set. The lock file pins
    # this moving branch to an exact revision for reproducible development.
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, utils, ... }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        gst-plugins-rs = pkgs.gst_all_1.gst-plugins-rs;
        gst-plugins-rs-livekit =
          assert pkgs.lib.assertMsg
            (pkgs.lib.versionAtLeast gst-plugins-rs.version "0.15.3")
            "eq2-cameras requires gst-plugins-rs 0.15.3 or newer for LiveKit protocol compatibility";
          (gst-plugins-rs.override {
            # The upstream WebRTC plugin has a Meson-level dependency on the
            # Rust RTP plugin, so these must be built as a pair.
            plugins = [
              "rtp"
              "webrtc"
            ];
            enableDocumentation = false;
          }).overrideAttrs (old: {
            mesonFlags = old.mesonFlags ++ [
              "-Dwebrtc-livekit=enabled"
            ];

            # Nixpkgs excludes the WebRTC plugin from its checked default set
            # because its upstream tests are not currently reliable.
            doCheck = false;
          });

        gst-deps = with pkgs.gst_all_1; [
          gstreamer
          gst-plugins-base
          gst-plugins-good
          gst-plugins-bad
          gst-plugins-ugly
          gst-libav
        ] ++ [
          gst-plugins-rs-livekit
          pkgs.libnice.out
        ];

        backendShell = pkgs.mkShell {
          packages = with pkgs; [
            python3
            uv
            livekit
            ruff
            livekit-cli
          ];

          # uv uses the Python interpreter supplied by this shell.
          UV_PYTHON = "${pkgs.python3}/bin/python3";
        };
      in
      {
        devShells = {
          cameras = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              pkg-config
              clang
              rustc
              cargo
              gst_all_1.gst-plugins-base
              livekit
              livekit-cli
            ];

            buildInputs = with pkgs; [
              glib
              libclang.lib
            ] ++ gst-deps;
          };
        } // pkgs.lib.optionalAttrs pkgs.stdenv.hostPlatform.isLinux {
          backend = backendShell;
          default = backendShell;
        };
      });
}
