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

        makeGstPluginsRsLivekit = packageSet:
          let
            gst-plugins-rs = packageSet.gst_all_1.gst-plugins-rs;
          in
          assert packageSet.lib.assertMsg
            (packageSet.lib.versionAtLeast gst-plugins-rs.version "0.15.3")
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

        makeCameraPackage = packageSet:
          let
            gst-plugins-rs-livekit = makeGstPluginsRsLivekit packageSet;
            gst-deps = with packageSet.gst_all_1; [
              gstreamer
              gst-plugins-base
              gst-plugins-good
              gst-plugins-bad
              gst-plugins-ugly
              gst-libav
            ] ++ [
              gst-plugins-rs-livekit
              packageSet.libnice.out
              # Libcamera provides the libcamerasrc plugin for Raspberry Pi
              # CSI sensors such as the IMX708.
              packageSet.libcamera
            ];
          in
          packageSet.rustPlatform.buildRustPackage {
            pname = "eq2-cameras";
            version = "0.1.0";

            # Avoid copying Cargo's local build directory into the derivation.
            src = packageSet.lib.cleanSource ./onboard/cameras;
            cargoLock.lockFile = ./onboard/cameras/Cargo.lock;

            # These are programs executed by the build machine. buildPackages
            # keeps them x86_64 when this package is cross-compiled.
            nativeBuildInputs = [
              packageSet.buildPackages.pkg-config
              packageSet.buildPackages.makeWrapper
            ];
            buildInputs = [ packageSet.glib ] ++ gst-deps;

            # GStreamer discovers codecs and the LiveKit sink dynamically, so
            # make every required plugin directory explicit at runtime.
            postFixup = ''
              wrapProgram $out/bin/eq2-cameras \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${packageSet.gst_all_1.gstreamer}/lib/gstreamer-1.0" \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${packageSet.libcamera}/lib/gstreamer-1.0" \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${packageSet.gst_all_1.gst-plugins-base}/lib/gstreamer-1.0" \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${packageSet.gst_all_1.gst-plugins-good}/lib/gstreamer-1.0" \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${packageSet.gst_all_1.gst-plugins-bad}/lib/gstreamer-1.0" \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${packageSet.gst_all_1.gst-plugins-ugly}/lib/gstreamer-1.0" \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${packageSet.gst_all_1.gst-libav}/lib/gstreamer-1.0" \
                --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "${gst-plugins-rs-livekit}/lib/gstreamer-1.0"
            '';
          };

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
        packages = {
          eq2-cameras = makeCameraPackage pkgs;
        } // pkgs.lib.optionalAttrs (system == "x86_64-linux") {
          # A runnable ARM64 Linux package built from an x86_64 Linux builder.
          eq2-cameras-aarch64 = makeCameraPackage pkgs.pkgsCross.aarch64-multiplatform;
        };

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
            ] ++ (let
              gst-plugins-rs-livekit = makeGstPluginsRsLivekit pkgs;
            in
              (with pkgs.gst_all_1; [
                gstreamer
                gst-plugins-base
                gst-plugins-good
                gst-plugins-bad
                gst-plugins-ugly
                gst-libav
              ]) ++ [
                gst-plugins-rs-livekit
                pkgs.libnice.out
              ] ++ pkgs.lib.optional pkgs.stdenv.hostPlatform.isLinux pkgs.libcamera);
          };
        } // pkgs.lib.optionalAttrs pkgs.stdenv.hostPlatform.isLinux {
          backend = backendShell;
          default = backendShell;
        };
      });
}
