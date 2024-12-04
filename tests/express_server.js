const express = require('express');
const path = require('path');
const compression = require('compression');
const serveStatic = require('serve-static');
const cluster = require('cluster');
const os = require('os');

const numCPUs = os.cpus().length;
const port = 4321;
const directory = path.join(__dirname, '../test_resources');

if (cluster.isMaster) {
    // Fork workers
    for (let i = 0; i < numCPUs; i++) {
        cluster.fork();
    }

    cluster.on('exit', (worker, code, signal) => {
        console.log(`Worker ${worker.process.pid} died`);
        cluster.fork(); // Restart the worker
    });
} else {
    const app = express();

    // Enable gzip compression
    app.use(compression());

    // Serve static files with caching
    app.use(serveStatic(directory, {
        maxAge: '1d', // Cache static files for 1 day
        etag: false
    }));

    app.get('*', (req, res) => {
        res.sendFile(path.join(directory, req.path));
    });

    app.listen(port, '127.0.0.1', () => {
        console.log(`Worker ${process.pid} is running at http://127.0.0.1:${port}`);
    });
}