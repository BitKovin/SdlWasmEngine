import http.server
import socketserver
import os

PORT = 8779

class CORSRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        os.chdir(script_dir)
        super().__init__(*args, **kwargs)

    def end_headers(self):
        # Required for SharedArrayBuffer / Emscripten pthreads
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        # Optional for cross-origin resources (if needed)
        self.send_header("Access-Control-Allow-Origin", "*")
        super().end_headers()

    def guess_type(self, path):
        if path.endswith(".wasm"):
            return "application/wasm"
        if path.endswith(".js"):
            return "text/javascript"
        if path.endswith(".data"):
            return "application/octet-stream"
        return super().guess_type(path)

    def translate_path(self, path):
        path = super().translate_path(path)
        return os.path.normpath(path)

if __name__ == "__main__":
    print(f"Serving files from: {os.getcwd()}")
    print(f"Starting server at http://localhost:{PORT}")

    try:
        with socketserver.TCPServer(("", PORT), CORSRequestHandler) as httpd:
            print("Server ready with COOP/COEP headers")
            print("Press Ctrl+C to stop")
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped")
    except OSError as e:
        print(f"Error: {e}")
        print("Try changing the port number if you get 'Address already in use'")
