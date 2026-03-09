#GNU General Public License v3.0
#Code by MegaKG

import Pages

class e404(Pages.webpage):
    def connect(self,Request: Pages.Connection):
        Request.sendCode(404)
        Request.sendLength(54)
        Request.sendType('text/html')

        Request.print("<html><body><h3>404, Page Not Found</h3></body></html>")

class e500(Pages.webpage):
    def connect(self,Request: Pages.Connection):
        Request.sendCode(500)
        Request.sendType('text/html')

        Request.print("<html><body><h3>500, Internal Server Error</h3></body></html>")

class e401(Pages.webpage):
    def connect(self,Request: Pages.Connection):
        Request.sendCode(401)
        Request.sendType('text/html')

        Request.print("<html><body><h3>401, Not Authorised</h3></body></html>")