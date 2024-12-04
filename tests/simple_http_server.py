import http.server
import socketserver

PORT = 4321
DIRECTORY = "../test_resources"

class MyHttpRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

Handler = MyHttpRequestHandler

with socketserver.TCPServer(("127.0.0.1", PORT), Handler) as httpd:
    print("Serving at port", PORT)
    httpd.serve_forever()