import { useState, useEffect } from "react";
import { LiveKitRoom } from "@livekit/components-react";
import { CameraFeeds } from "./Cameras";
const Cameras = () => {
  type TokenCredentials = {
    server_url: string;
    participant_token: string;
  };

  const [credentials, setCredentials] = useState<TokenCredentials | null>(null);
  const [error, setError] = useState<string | null>(null);

  async function connect() {
    try {
      const response = await fetch('http://127.0.0.1:8080/token', {
        method: 'POST',
      });

      if (!response.ok) {
        throw new Error(`Token request failed (${response.status})`);
      }

      const credentials = await response.json();
      setCredentials(credentials);
    } catch (error) {
      setError(error instanceof Error ? error.message : 'Unable to request a LiveKit token');
    }
  }

  useEffect(() => {
    connect();
  }, []);

  return (
    <>
      {credentials && (
        <LiveKitRoom
          serverUrl={credentials.server_url}
          token={credentials.participant_token}
          onError={(error) => setError(`LiveKit connection failed: ${error.message}`)}
        >
          <CameraFeeds />
        </LiveKitRoom>
      )}
    </>
  );
}

export { Cameras };
