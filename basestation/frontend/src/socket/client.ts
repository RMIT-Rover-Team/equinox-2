// responsible for websocket connection

type Listener = () => void; // callback function

/** Creates a single topic which stores the attached listeners and the latest message.
 * Reconnects automatically unless manually closing the websocket.
 * @param url Full URL for the topic's websocket (example ws://127.0.0.1:8000/ws/fooTopic)
 */
function createTopic(url: string) {
  const listeners = new Set<Listener>();
  let latest: unknown;
  let ws: WebSocket;
  let closed = false;
  let attempt = 0;
  let reconnectTimer: ReturnType<typeof setTimeout> | undefined;

  /** Opens a websocket and the corresponding logic for the websocket. */
  function open() {
    ws = new WebSocket(url);
    ws.onopen = () => { attempt = 0; };

    ws.onmessage = (event: MessageEvent) => {
      try {
        latest = JSON.parse(event.data);
      } catch {
        console.warn(`Ignoring malformed message on ${url}`);
        return;
      }
      listeners.forEach((listener) => listener()); // run callback
    };

    ws.onclose = (event: CloseEvent) => {
      console.log('Close code: ' + event.code);
      console.log('Close reason: ' + event.reason);
      if (closed) return;  // if intentionally closed, ignore.

      const delay = Math.min(500 * 2 ** attempt++, 10_000);
      reconnectTimer = setTimeout(() => {
        if (!closed) open();
      }, delay);
    };
  }

  open();

  /**
   * Gets the latest JSON message from the topic.
   * @return The last parsed message for the topic, or `undefined`
   *          if no message has arrived yet.
   */
  function get(): unknown { return latest; }

  /**
   * Send data to websocket topic.
   * @param msg The JSON message to send. Ensure it is not stringified.
   * @return Whether the browser executed the send operation successfully.
   *                   Note this does not account for whether the message was received successfully.
   */
  function send(msg: object): boolean {
    if (ws.readyState !== WebSocket.OPEN) return false;
    ws.send(JSON.stringify(msg));
    return true;
  }

  /**
   * Closes the websocket. Cancels any pending reconnections and halts future reconnections.
   */
  function close(): void {
    closed = true;
    clearTimeout(reconnectTimer);
    ws.close();
  }

  return { listeners, get, send, close, };
}

type Topic = ReturnType<typeof createTopic>;

/**
 * Initializes the websocket client and the associated websocket functions.
 * @param baseUrl Base websocket URL address (for example ws://127.0.0.1)
 * @return Client functions
 */
export function createSocketClient(baseUrl: string) {
  const topics = new Map<string, Topic>();

  /**
   * Subscribes to a topic. If the topic does not exist, the topic is created.
   * When the component unmounts, the listener is removed from the topic automatically.
   * @param topic The topic to subscribe to.
   * @param onChange A callback function to call when the topic subscribed to is updated.
   * @return Returns a function for unsubscribing the component.
   *
   * Example of subscribing, handing off the unsubscribe to React to automatically unmount.
   * @example
   * ```ts
   * useEffect(() => {
   * const onFoo = () => {
   *   const foo = client.get('fooTopic') as DataType | undefined;
   *   // use foo data however you want
   * }
   * return client.subscribe('fooTopic', onFoo);
   * }, [client]);
   * ```
   * Example of subscribing, manually assigning an unsubscribe function.
   * @example
   * ```ts
   * const unsubscribe = client.subscribe('fooTopic', () => console.log(client.get('fooTopic')));
   * // later
   * unsubscribe();
   * ```
   */
  function subscribe(topic: string, onChange: Listener): () => void {
    let entry = topics.get(topic);

    if (!entry) {
      entry = createTopic(`${baseUrl}/ws/${topic}`);
      topics.set(topic, entry);
    }

    entry.listeners.add(onChange);

    return () => {
      entry.listeners.delete(onChange);
      if (entry.listeners.size === 0) {
        entry.close();
        topics.delete(topic);
      }
    };
  }

  /**
   * Gets the latest JSON object from the topic, if it exists.
   * @param topic The topic to get the latest data from.
   * @return The last parsed message for the topic, or `undefined`
   *                    if the topic has no subscribers or no message has arrived yet.
   */
  function get(topic: string): unknown {
    return topics.get(topic)?.get();
  }

  /**
   * Sends a JSON object to the topic.
   * @param topic The topic to send data to
   * @param msg The JSON msg to send. Make sure the JSON object is not stringified.
   * @return `true` if the message was able to be sent, and `false` if otherwise.
   */
  function send(topic: string, msg: object): boolean {
    return topics.get(topic)?.send(msg) ?? false;
  }

  /**
   * Closes all topics and their associated websocket connections.
   */
  function close(): void {
    topics.forEach((entry) => entry.close());
    topics.clear();
  }

  return { subscribe, get, send, close };
}

export type SocketClient = ReturnType<typeof createSocketClient>;
