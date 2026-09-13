

import uvicorn

from .app import app




def main():
    uvicorn.run(app, host="127.0.0.1", port=8080)

if __name__ == "__main__":
    main()
