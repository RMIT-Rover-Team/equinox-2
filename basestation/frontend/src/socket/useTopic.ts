import { useCallback, useContext, useSyncExternalStore } from 'react';
import { SocketContext } from './SocketContext';

/**
 * Hook to get data from a topic. Wraps useSyncExternalStore with subscribe and snapshot functions.
 * @param topic Name of topic to use.
 * @returns Returns the JSON object if found, otherwise returns undefined.
 */
export function useTopic<T>(topic: string): T | undefined {
	const client = useContext(SocketContext);
	if (!client) throw new Error('useTopic must be used inside <SocketProvider>');

	const subscribe = useCallback(
		(onChange: () => void) => client.subscribe(topic, onChange),
		[client, topic],
	);

	return useSyncExternalStore(subscribe, () => client.get(topic) as T | undefined);
}
