#GNU General Public License v3.0
#Code by MegaKG
import socket
import ssl
import TCPstreams5 as tcp

#sudo openssl req -x509 -nodes -days 1095 -newkey rsa:2048 -out /etc/apache2/ssl/server.crt -keyout /etc/apache2/ssl/server.key


class wrappedServer(tcp.serverCon):
        def __init__(self,tcpcon,KEY,CERT):
                ssl_sock = ssl.wrap_socket(tcpcon.conn,
                                 server_side=True,
                                 certfile=CERT,
                                 keyfile=KEY)
                
                self.conn = ssl_sock
                self.TLSinfo = {
                        'Cipher':ssl_sock.cipher(),
                        'Peer':repr(ssl_sock.getpeername())
                        }
                self.info = tcpcon.info


class wrappedClient(tcp.clientCon):
        def __init__(self,tcpcon,CERT=None):
                if CERT != None:
                        ssl_sock = ssl.wrap_socket(tcpcon.conn,
                                   ca_certs=CERT,
                                   cert_reqs=ssl.CERT_REQUIRED)
                else:
                        ssl_sock = ssl.wrap_socket(tcpcon.conn)
                
                self.TLSinfo = {
                        'Cipher':ssl_sock.cipher(),
                        'Peer':repr(ssl_sock.getpeername())
                        }
                self.conn = ssl_sock
                self.info = tcpcon.info


def testClient():
	con = tcp.clientCon('google.com',443)
	wr = wrappedClient(con,None)
	wr.senddat(b'GET / HTTP/1.1\n\n')
	print(wr.getdat(4096))
                
