import { useEffect, useState } from 'react';
import type { ReactNode } from 'react';
import { createSocketClient } from './client';
import { SocketContext } from './SocketContext';

type SocketProviderProps = {
	baseUrl: string;
	children: ReactNode;
};

export function SocketProvider({ baseUrl, children }: SocketProviderProps) {
	const [client] = useState(() => createSocketClient(baseUrl));

	useEffect(() => {
		return () => client.close();
	}, [client]);

	return <SocketContext value={client}>{children}</SocketContext>;
}
