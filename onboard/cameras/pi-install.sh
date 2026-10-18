#!/usr/bin/env bash
# Install the eq2-cameras bundle on 64-bit Raspberry Pi OS (trixie).
# Works offline: uses the Pi's apt-installed GStreamer and libcamera, and only
# adds the bundled .debs, our binary, and the LiveKit GStreamer plugins.
set -euo pipefail

bundle_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bin_dir="$HOME/.local/bin"
# GStreamer scans this per-user directory automatically; no env vars needed.
plugin_dir="${XDG_DATA_HOME:-$HOME/.local/share}/gstreamer-1.0/plugins"

echo "==> Installing bundled packages"
sudo apt-get install -y --no-download "$bundle_dir"/debs/*.deb

echo "==> Installing eq2-cameras and plugins"
mkdir -p "$bin_dir" "$plugin_dir"
install -m 755 "$bundle_dir/bin/eq2-cameras" "$bin_dir/"
install -m 644 "$bundle_dir"/plugins/*.so "$plugin_dir/"

echo "==> Checking GStreamer elements"
missing=0
for element in livekitwebrtcsink nicesrc libcamerasrc capsfilter videoconvert h264parse valve; do
  if ! gst-inspect-1.0 "$element" >/dev/null 2>&1; then
    echo "    missing: $element" >&2
    missing=1
  fi
done
if (( missing )); then
  echo "Some elements are missing; see above." >&2
  exit 1
fi

if ! id -nG | grep -qw video; then
  echo "Note: add yourself to the video group for camera access:"
  echo "  sudo usermod -aG video $USER   (then log out and back in)"
fi

cat <<EOF

Installed $bin_dir/eq2-cameras. To run:
  cp $bundle_dir/eq2-cameras.env.example ~/eq2-cameras.env   # then edit it
  set -a; . ~/eq2-cameras.env; set +a
  RUST_LOG=info eq2-cameras
EOF
