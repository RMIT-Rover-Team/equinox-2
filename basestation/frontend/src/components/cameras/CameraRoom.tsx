import { useEffect, useState } from 'react';
import type { ReactNode } from 'react';
import { LiveKitRoom } from '@livekit/components-react';

const TOKEN_URL = 'http://127.0.0.1:8080/token';

type Credentials = { server_url: string; participant_token: string };

function CameraRoom({ children }: { children: ReactNode }) {
	const [credentials, setCredentials] = useState<Credentials | null>(null);

	useEffect(() => {
		fetch(TOKEN_URL, { method: 'POST' })
			.then((response) => response.json())
			.then(setCredentials)
			.catch((error) => console.error('Camera token request failed', error));
	}, []);

	return (
		<LiveKitRoom
			serverUrl={credentials?.server_url}
			token={credentials?.participant_token}
			connect={credentials !== null}
			audio={false}
			video={false}
		>
			{children}
		</LiveKitRoom>
	);
}

export default CameraRoom;
