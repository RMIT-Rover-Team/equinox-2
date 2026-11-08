import { useContext, useSyncExternalStore } from "react";
import SocketContext from "./SocketContext"
function useTopic<T>(topic: string, intiial: T): T {
  const client = useContext(SocketContext)!;
  return useSyncExternalStore(
    (onChange) => client.subscribe(topic, onChange), () => { client.latest(topic) ?? initial); }
  );
}
