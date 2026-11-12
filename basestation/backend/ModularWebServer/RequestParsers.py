#GNU General Public License v3.0
#Code by MegaKG

class decodedRequest:
    def __init__(self,headerDictionary, variableDictionary,cookieDictionary,requestedResource):
          self.VariableDictionary = variableDictionary
          self.CookieDictionary = cookieDictionary
          self.RequestedResource = requestedResource
          self.HeaderDictionary = headerDictionary

          #print('RES',requestedResource)
          #print('HEAD',headerDictionary)
          #print('COOK',cookieDictionary)
          #print('VARS',variableDictionary)


    def getResource(self):
         return self.RequestedResource
    
    def getVariable(self,key):
         return self.VariableDictionary.get(key)
    
    def getVariableNames(self):
         return self.VariableDictionary.keys()
    
    def getCookie(self,key):
         return self.CookieDictionary.get(key)
    
    def getCookieNames(self):
         return self.CookieDictionary.keys()
    
    def getHeader(self,key):
         return self.HeaderDictionary.get(key)
    
    def getHeaderNames(self):
         return self.HeaderDictionary.keys()

    

class RequestParser:
    def __init__(self):
        pass

    #This processes GET requests
    def procGet(self,GETR):
        RET = {}
        for pair in GETR.split('&'):
                    SP = pair.split('=')
                    RET[SP[0]] = SP[1].replace('+',' ')
        return RET
    
    #Process Vars
    def procVar(self,VARR):
        RET = {}
        for pair in VARR.split(';'):
                    if pair[0] == ' ':
                         pair = pair[1:]
                    if pair[-1] == ' ':
                         pair = pair[:-1]

                    if '=' in pair:
                        SP = pair.split('=')
                        RET[SP[0]] = SP[1].replace('+',' ')
                    else:
                        RET[pair] = None
        return RET

    #Merges two dictionaries, internal method
    def _dictMerge(self,D1,D2):
        for key in D1:
            D2[key] = D1[key]
        return D2

    #This parses HTTP request headers from the client
    def parseRequest(self,Req):
        #First Get the Request type and resource
        #(Req)
        SP = Req.replace(b'\r',b'').split(b'\n')
        Resource = SP[0].decode()

        #Process HTTP Attributes in the request
        HEADERS = {}
        VARIABLES = {}
        COOKIES = {}

        #Process GET values
        Resource = Resource.split(' ')[1]
        if '?' in Resource:
            Split = Resource.split('?')
            if len(Split) == 2:
                Resource, GET = Split

                if len(GET) > 0:
                    VARIABLES = self.procGet(GET)
            else:
                 Resource = Split[0]


        #Process POST values, header variables and cookies 
        for i in SP[1:]:
            #print(i)
            if (len(i) > 3) and (b':' in i):
                REQ_SP = i.split(b': ')
                if (REQ_SP[0] == b'Cookie'):
                    COOKIES = self._dictMerge(COOKIES,self.procVar(REQ_SP[1].decode()))
                else:
                    HEADERS[REQ_SP[0].decode()] = REQ_SP[1].decode()
            elif (b'=' in i) and (b':' not in i):
                VARIABLES = self._dictMerge(VARIABLES,self.procGet(i.decode()))

        

        return decodedRequest(HEADERS,VARIABLES,COOKIES,Resource)