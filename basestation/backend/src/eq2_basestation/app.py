import asyncio
import os
from contextlib import asynccontextmanager, suppress
from uuid import uuid4

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from livekit import api

room_name = "rover"


@asynccontextmanager
async def lifespan(app: FastAPI):
    livekit = await asyncio.create_subprocess_exec(
        "livekit-server", "--dev", "--bind", "0.0.0.0", start_new_session=True
    )
    try:
        yield  # fastapi serves requests until shutdown starts.
    finally:
        if livekit.returncode is None:  # KILL
            with suppress(ProcessLookupError):
                livekit.terminate()
            try:
                _ = await asyncio.wait_for(livekit.wait(), timeout=5)
            except TimeoutError:
                with suppress(ProcessLookupError):
                    livekit.kill()
                _ = await livekit.wait()


app = FastAPI(lifespan=lifespan)

origins = [
    "http://localhost",
    "http://localhost:5173"
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["post, get"],
    allow_headers=["*"],
)

@app.get("/healthz")
async def health():
    return {"backend": "ok"}  # Check HTTP


@app.post("/token")
async def token():
    # each token generated requires a unique identifier.
    # i chose uuid for convenience but consider
    # something human readable later for debug purposes
    token = (
        api.AccessToken(  # subscribe only token
            os.environ["LIVEKIT_API_KEY"],
            os.environ["LIVEKIT_API_SECRET"],
        )
        .with_identity(f"viewer-{uuid4()}")
        .with_name("name")
        .with_grants(
            api.VideoGrants(
                room_join=True,
                room=room_name,
                can_subscribe=True,
                can_publish=False,
                can_publish_data=False,
            )
        )
        .to_jwt()
    )

    return {
        "server_url": "ws://127.0.0.1:7880",
        "participant_token": token,
    }
