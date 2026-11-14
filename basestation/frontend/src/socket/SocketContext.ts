import { createContext } from 'react';
import type { SocketClient } from './client';

export const SocketContext = createContext<SocketClient | null>(null);
