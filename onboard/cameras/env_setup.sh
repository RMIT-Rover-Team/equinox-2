export LIVEKIT_WS_URL='ws://127.0.0.1:7880'

export LIVEKIT_AUTH_TOKEN="$(
  lk token create \
    --api-key devkey \
    --api-secret secret \
    --identity rover-publisher \
    --join \
    --valid-for 24h \
    --room rover \
    --grant '{"canPublish":true,"canSubscribe":false}' \
    --token-only
)"

cargo run
