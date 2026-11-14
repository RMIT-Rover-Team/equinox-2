# Basestation frontend

React + TypeScript + Vite app for the Equinox 2 operator station. Camera feeds are received over LiveKit (WebRTC).

## Setup

Install the tools once (macOS):

```bash
curl -fsSL https://bun.sh/install | bash
brew install uv livekit livekit-cli ffmpeg
```

Then install the frontend's packages:

```bash
cd basestation/frontend
bun install
```

## Running

Each step runs in its own terminal tab. Leave them all running.

### 1. Backend and LiveKit

```bash
cd basestation/backend
bash run_backend.sh
```

This starts the token server on port 8080 and LiveKit on port 7880.

### 2. Frontend

```bash
cd basestation/frontend
bun run dev
```

Open http://localhost:5173. It must run on port 5173, because the backend only gives tokens to that address. If Vite says the port is in use, stop whatever else is on it (such as `cameras-mvp`).

### 3. Camera feeds

On the rover, `eq2-cameras` on the Pi sends the feeds. To test without the rover, send cameras plugged into device with `ffmpeg` and `lk`. The empty grid slots show "No signal" until a feed arrives.

Find each camera's number:

```bash
ffmpeg -hide_banner -f avfoundation -list_devices true -i ""
```

Each camera needs two tabs. Start `ffmpeg` first, because it waits for `lk` to connect. For camera `1`:

```bash
ffmpeg -f avfoundation -framerate 25 -video_size 640x480 -i "1" \
  -c:v libx264 -preset ultrafast -tune zerolatency -x264-params sliced-threads=0 \
  -profile:v baseline -pix_fmt yuv420p -bf 0 -g 50 -b:v 2M \
  -bsf:v h264_mp4toannexb -listen 1 -f h264 tcp://127.0.0.1:16401
```

```bash
lk room join --url ws://127.0.0.1:7880 --api-key devkey --api-secret secret \
  --identity cam-1 --publish h264://127.0.0.1:16401 rover
```

For more cameras, change the camera number (`-i "2"`), the port (`16402`) and the identity (`cam-2`) in both commands.

To test without any cameras, publish a demo video:

```bash
lk room join --url ws://127.0.0.1:7880 --api-key devkey --api-secret secret \
  --identity demo-1 --publish-demo rover
```

To check which feeds have joined, run the following. Each camera should show `tracks: 1`.

```bash
lk room participants list rover --url http://127.0.0.1:7880 --api-key devkey --api-secret secret
```

### Troubleshooting

| Problem                                                      | Fix                                                                                                                                     |
| ------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------- |
| `dial tcp [::1]:7880: connect: connection refused`           | Use `127.0.0.1`, not `localhost`. LiveKit only listens on IPv4. If it still fails, the backend isn't running.                           |
| `Selected framerate ... is not supported by the device`      | Use one of the modes ffmpeg lists after the error for `-framerate` and `-video_size`.                                                   |
| Only the top strip of the video shows, and the rest is green | Keep `-x264-params sliced-threads=0`. Without it, `lk` shows only the first slice of each frame.                                        |
| Slot stays black                                             | A feed only appears from its next full frame, every 2 seconds with `-g 50`. If it stays black, restart that camera's `ffmpeg` and `lk`. |

## Other scripts

| Command             | Does                                                                                 |
| ------------------- | ------------------------------------------------------------------------------------ |
| `bun run build`     | Type-checks and builds to `dist/`                                                    |
| `bun run lint`      | Runs oxlint                                                                          |
| `bun run test`      | Runs story tests in Chromium. Run `bunx playwright install chromium` the first time. |
| `bun run storybook` | Opens Storybook on port 6006                                                         |
