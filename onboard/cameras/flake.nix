                                                                                                                                     {
  description = "Equinox 2 Camera Streamer & WebRTC Service Environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-26.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        gst-plugins-rs-livekit =
          (pkgs.gst_all_1.gst-plugins-rs.override {
            plugins = [
              "rtp"
              "webrtc"
            ];
            enableDocumentation = false;
          }).overrideAttrs (old: {
            mesonFlags = old.mesonFlags ++ [
              "-Dwebrtc-livekit=enabled"
            ];

            doCheck = false;
            doInstallCheck = false;
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
      in
      {
        devShells.default = pkgs.mkShell {
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
      });
}
