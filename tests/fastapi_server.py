from fastapi import FastAPI
from fastapi.responses import FileResponse
import os

app = FastAPI()
directory = "../test_resources"

@app.get("/{filename:path}")
async def serve_file(filename: str):
    file_path = os.path.join(directory, filename)
    if os.path.exists(file_path):
        return FileResponse(file_path)
    return {"error": "File not found"}

if __name__ == '__main__':
    import uvicorn
    uvicorn.run(
        app,
        host="127.0.0.1",
        port=4321,
        workers=4,
        log_level="critical",  # Minimize logging
        access_log=False       # Disable access log
    )