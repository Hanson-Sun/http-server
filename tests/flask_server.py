from flask import Flask, send_from_directory
import os

app = Flask(__name__)
directory = "../test_resources"

@app.route('/<path:filename>')
def serve_file(filename):
    return send_from_directory(directory, filename)

if __name__ == '__main__':
    from gevent.pywsgi import WSGIServer
    http_server = WSGIServer((host, port), app)
    http_server.serve_forever()