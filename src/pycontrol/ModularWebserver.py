#!/usr/bin/env python3
#GNU General Public License v3.0
#Code by MegaKG

#Import some of the custom support libraries
import TCPstreams5 as tcp #This handles TCP sockets nicely
import TLSwrapper2 as tls #This converts TCP sockets from the above library to SSL, providing the same interface
import WebsocketUtils #Handler for Websocket magic 
import DefaultErrors #Web Pages for Default Errors
import RequestParsers #Parses POST and GET Requests
import Pages

#Import standard library things
import threading #Used to start handlers for clients after they have connected
import time #Used to get Current Epoch time in seconds


#The main class, initialised as follows:
#Server = webServer(Dict)
#Where Dict is a Dictionary containing required configuration values
#PassIn is an optional argument that can be used to pass objects to webpage workers (Intended for use if this server is run as a library)
class webServer:
    #The Initialisation code of the class / webserver object
    def __init__(self,HostIP='0.0.0.0',HostPort=8080,TLS=False, TLS_Cert=None, TLS_Key=None, MaxConnections=100,
                 MaxConnectionTime=5, AllowWebsocket=True, MaxWebsocketTime=10000, PageMap={}, CatchAll=None):
        #The first thing to do is initialise the logger so error messages have somewhere to go
        print("Initiate Server")

        #Save the Config internally
        self.HostIP = HostIP
        self.HostPort = HostPort
        self.TLS = TLS
        self.TLS_Cert = TLS_Cert
        self.TLS_Key = TLS_Key
        if ((self.TLS_Cert is None) or (self.TLS_Keys is None)) and self.TLS:
            print("Error, TLS certificates not provided, disabling")
            self.TLS = False
        self.MaxConnections = MaxConnections
        self.MaxConnectionTime = MaxConnectionTime
        self.AllowWebsocket = AllowWebsocket
        self.MaxWebsocketTime = MaxWebsocketTime
        self.PageObjects = PageMap
        self.CatchallPage = CatchAll


        #Open the TCP server socket on specified Address and Port
        self.server = tcp.newServer(HostIP,HostPort)

        #These keep track of connnected clients (and their threads)
        self.connections = []
        self.connectionCount = 0

        #Create the parser
        self.Parser = RequestParsers.RequestParser()

    #This function handles a connected client
    def client(self,CON,ID):
        IDENTIFIER = str(ID) + ' -> ' + str(CON.report()['Address'])
        try:
            #Upgrade to TLS if required
            if self.TLS:
                CON = tls.wrappedServer(CON,self.TLS_Key,self.TLS_Cert)

            #Repeat while the connection is kept alive
            while True:
                #Read our Request
                if CON.report()['Alive'] == False:
                    print(IDENTIFIER, "Disconnected")
                    break

                try:
                    Request = CON.getdat(1024)
                except Exception as E:
                    break

                if (Request == b'') or (Request == False):
                    print(IDENTIFIER, "Connection Died")
                    break


                #print(IDENTIFIER, "Got Request")
                #Parse the raw request and get both the resource and the variable values
                try:
                    ParsedOptions = self.Parser.parseRequest(Request)
                except Exception as E:
                    print("Header Error",E)
                    break

                try:
                    #Keep a record of this transaction
                    print(IDENTIFIER,"has requested",ParsedOptions.getResource())

                    #Generate Connection Object
                    Connection = Pages.Connection(CON,ParsedOptions)

                    #Determine type of Response
                    #First check if the resource exists
                    if ParsedOptions.getResource() in self.PageObjects:
                        #Get the Resource
                        MyPage = self.PageObjects[ParsedOptions.getResource()]

                        #Handle Websocket if needed
                        if 'Upgrade' in ParsedOptions.getHeaderNames():
                            #If a websocket is wanted
                            if ParsedOptions.getHeader('Upgrade') == 'websocket':
                                if self.AllowWebsocket:
                                    print(IDENTIFIER,"Upgrading to Websocket")
                                    #Do the special magic that prevents Caching
                                    HEADER = WebsocketUtils.responseHeader(ParsedOptions)
                                    CON.sendstdat(HEADER)

                                    #Convert CON to something usable
                                    CON = WebsocketUtils.wrappedConnection(CON)
                                    Connection = Pages.Connection(CON,ParsedOptions)

                                    #Now we begin websocket
                                    try:
                                        MyPage.websocket(Connection)
                                    except OSError:
                                        print(IDENTIFIER,"Websocket Died")
                                else:
                                    print(IDENTIFIER,"Deny Websocket")

                        #Otherwise Handle HTTP
                        else:
                            #Respond with the requested resource

                            MyPage.connect(Connection)

                    #Otherwise, handle it
                    else:
                        
                        if self.CatchallPage is not None:
                            #Server Catchall page
                            print(IDENTIFIER,"Redirect to Catchall")
                            self.CatchallPage.connect(Connection)

                        else:
                            #The Default 404 Handler
                            print(IDENTIFIER,"404 for",ParsedOptions.getResource())
                            page = DefaultErrors.e404(None)

                            page.connect(Connection)

                except Exception as E:
                    #The Default 500 Handler
                    print(ID,"ERROR 500",E)
                    page = DefaultErrors.e500(None)

                    page.connect(Connection)
                    raise
                    

        except tls.ssl.SSLError:
            print(ID,"CertError")

        #Clean Up
        CON.close()
        print(IDENTIFIER,"Cleanup")


    #This is the main loop of the webserver
    def run(self):

        #For logging, each client has an ID per connection
        IDcounter = 0
        while True:
            #First wait for an available position (If the server is full)
            while self.connectionCount > self.MaxConnections:
                #Iterate over all threads, killing those that should be gone
                for count in range(len(self.connections)):
                    con = self.connections[len(self.connections)-1-count]
                    Report = con['con'].report()
                    #Here we check how long the connection is on and if the system reports it as alive, thus preventing DoS attacks
                    if (not con['thread']) or (not Report['Alive']) or ((time.time() - Report['InitTime']) > self.MaxConnectionTime ):
                        print(str(con['ID']),"Connection Terminate")
                        del self.connections[len(self.connections)-1-count]
                        self.connectionCount -= 1

                time.sleep(0.1)

            #Accept a Client
            Connection = tcp.serverCon(self.server)
            
            #Create a record
            self.connections.append(
                {
                    'con':Connection,
                    'thread':threading.Thread(target=self.client,args=(Connection,IDcounter),name='Worker'+str(IDcounter)),
                    'ID':IDcounter
                }
            )

            #Start the handler
            self.connections[-1]['thread'].start()
            print("Started Connection",IDcounter)

            IDcounter += 1

            





if __name__ == '__main__':
    import testPageLength
    import testPageNoLength
    import testSocket
    import testSocketLength
    import testPageData
    import test401Auth

    serv = webServer(PageMap = {
        '/':testPageLength.page(None),
        '/nl':testPageNoLength.page(None),
        '/sk':testSocket.page(None),
        '/socket':testSocket.page(None),
        '/lengthsock':testSocketLength.page(None),
        '/testdata':testPageData.page(None),
        '/basicauth':test401Auth.page(None)
    })
    serv.run()
