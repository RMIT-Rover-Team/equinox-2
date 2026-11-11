import { useCallback, useContext, useSyncExternalStore } from 'react';
import { SocketContext } from './SocketContext';

export function useTopic<T>(topic: string): T | undefined {
	const client = useContext(SocketContext);
	if (!client) throw new Error('useTopic must be used inside <SocketProvider>');

	const subscribe = useCallback(
		(onChange: () => void) => client.subscribe(topic, onChange),
		[client, topic],
	);

	return useSyncExternalStore(subscribe, () => client.get(topic) as T | undefined);
}
