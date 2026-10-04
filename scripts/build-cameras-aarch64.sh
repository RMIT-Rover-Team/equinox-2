#!/usr/bin/env bash
# Build the ARM64 camera application on x86_64 Linux and bundle its complete
# Nix closure for offline installation on an ARM64 Raspberry Pi.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output_dir="${1:-${repo_root}/dist/eq2-cameras-aarch64}"

if [[ "$(uname -s)" != "Linux" || "$(uname -m)" != "x86_64" ]]; then
  echo "This script must be run on an x86_64 Linux machine." >&2
  exit 1
fi

if ! command -v nix >/dev/null || ! command -v nix-store >/dev/null; then
  echo "Nix with the legacy nix-store command is required on the build machine." >&2
  exit 1
fi

mkdir -p "$output_dir"
output_dir="$(cd "$output_dir" && pwd)"
bundle_dir="$output_dir/eq2-cameras-aarch64-bundle"
archive="$bundle_dir/eq2-cameras-aarch64.closure.nar"

rm -rf "$bundle_dir"
mkdir -p "$bundle_dir"

cd "$repo_root"
echo "Building the ARM64 package..."
package_path="$(nix build --print-build-logs --print-out-paths .#eq2-cameras-aarch64)"

echo "Exporting the complete runtime closure..."
# Export every store path the wrapped executable needs, including GStreamer
# plugins and the dynamic loader. The Pi can import this archive offline.
nix-store --export $(nix-store --query --requisites "$package_path") > "$archive"

sha256sum "$archive" > "$archive.sha256"

cat > "$bundle_dir/import-and-run.sh" <<EOF
#!/usr/bin/env bash
# Run this on the ARM64 Raspberry Pi. Nix is the only prerequisite.
set -euo pipefail

bundle_dir="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" && pwd)"
nix-store --import < "\$bundle_dir/eq2-cameras-aarch64.closure.nar"
exec "$package_path/bin/eq2-cameras" "\$@"
EOF
chmod +x "$bundle_dir/import-and-run.sh"

cat > "$bundle_dir/README.md" <<EOF
# EQ2 cameras for ARM64 Linux

This bundle contains the complete Nix runtime closure for `eq2-cameras`,
including its GStreamer libraries and plugins. It was built for 64-bit ARM
Linux (for example, Raspberry Pi OS 64-bit or NixOS on a Raspberry Pi).

## On the Raspberry Pi

The only required pre-installation is Nix. Copy this whole directory to the Pi,
then run:

```sh
cd eq2-cameras-aarch64-bundle
sha256sum -c eq2-cameras-aarch64.closure.nar.sha256
./import-and-run.sh
```

Pass application arguments after the script name as usual. The script imports
the closure into `/nix/store` and then starts the wrapped executable. It is
safe to run again; Nix skips store paths already imported.

The program still needs its normal runtime configuration (such as
`LIVEKIT_WS_URL` and `LIVEKIT_AUTH_TOKEN`) and access to the Pi's camera
device. Those are application inputs, not build dependencies.
EOF

echo
echo "Bundle created: $bundle_dir"
echo "Copy that directory to the Raspberry Pi, then run ./import-and-run.sh there."
