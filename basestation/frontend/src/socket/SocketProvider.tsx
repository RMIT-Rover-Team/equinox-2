import { useEffect, useState } from 'react';
import type { ReactNode } from 'react';
import { createSocketClient } from './client';
import { SocketContext } from './SocketContext';

type SocketProviderProps = {
	url: string;
	children: ReactNode;
};

export function SocketProvider({ url, children }: SocketProviderProps) {
	const [client] = useState(() => createSocketClient(url));

	useEffect(() => {
		return () => client.close();
	}, [client]);

	return <SocketContext value={client}>{children}</SocketContext>;
}
