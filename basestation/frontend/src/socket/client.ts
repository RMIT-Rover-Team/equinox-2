// responsible for websocket connection

type Listener = () => void;

export function createSocketClient(url: string) {
  let ws: WebSocket | null = null;
  const listeners = new Map<string, Set<Listener>>();
  const latest = new Map<string, unknown>();

  function send(msg: object) {
    if (ws?.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify(msg));
    }
  }
  function handleMessage(event: MessageEvent) {
    const data: Object = JSON.parse(event.data);
    latest.set()

  function connect() {

  }
  function close() {

  }
}
