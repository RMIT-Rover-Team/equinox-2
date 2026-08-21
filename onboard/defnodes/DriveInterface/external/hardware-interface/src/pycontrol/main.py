#!/usr/bin/env python3
import ModularWebserver as MW
import wsControlPage as WSP
import sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 main.py <can interface>")
        exit(1)

    drivePage = WSP.page(sys.argv[1])

    serv = MW.webServer(PageMap = {
        '/':drivePage,
        '/driveChannel':drivePage
    },
        HostPort=5001)
    serv.run()