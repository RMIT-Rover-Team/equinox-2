import { useTracks, VideoTrack } from "@livekit/components-react";
import type { TrackReference } from "@livekit/components-react";
import { Track } from "livekit-client";
import { ConnectionQualityIndicator } from "@livekit/components-react";


const CameraFeeds = () => {
  const tracks: TrackReference[] = useTracks([
    Track.Source.Camera,
  ]).filter((track) => track.publication.kind === Track.Kind.Video);

  return (
    <>
      {tracks.map((track) => (
        <VideoTrack
          key={track.publication.trackSid}
          trackRef={track}
        />
      ))}
    </>
  );
}
export { CameraFeeds };
