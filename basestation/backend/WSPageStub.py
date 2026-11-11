from ModularWebServer import Pages
from ModularWebServer import WebsocketUtils

# A class to give the WS wrapper a nicer interface
class WSConnection:
    def __init__(self, CON: WebsocketUtils.wrappedConnection):
        self._connection = CON

    def send(self, data: str) -> None:
        V = self._connection.sendstdat(data)
        if not V:
            raise Exception("Lost Connection")

    def recv(self) -> str:
        V = self._connection.getstdat(8192)
        if not V:
            raise Exception("Lost Connection")
        return V

    def close(self) -> None:
        self._connection.close()


class wsPageStub(Pages.webpage):
    # Add additional variables to the init function
    def __init__(self,Options):
        super().__init__(Options)

        self.HandlerHooks = {}

    # Creates a handler mapping
    # Pass in topic as path, and function of form 
    def newHandler(self, topic: str, handlerFunction: function):
        self.HandlerHooks[topic] = handlerFunction


    def connect(self,Request: Pages.Connection):
        MSG = "<html>Hello World! - This is a websocket server!</html>"

        Request.sendCode(200)
        Request.sendLength(len(MSG))
        Request.sendType("text/html")

        Request.print(MSG)




    def websocket(self,Request: Pages.Connection):
        print("Socket Listener Running")
        CON = Request.getConnectionObject()

        # We get the path of the connection to find out what stub is connected
        path = Request.getRequestHeader().getResource()

        # Find the handler for the path
        if path in self.HandlerHooks:
            self.HandlerHooks[path](WSConnection(CON))

        # Otherwise we log an error
        else:
            print("No handler found for path: " + path)
        

        