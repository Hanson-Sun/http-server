# HTTP server

Yeah its literally just a basic HTTP server implementation in C++ using linux sockets...

Features:
- [x] GET, POST, PUT, DELETE requests
- [x] Basic routing
- [x] Static file serving

yeah thats it :frown2: 

Here are some benchmarks with `wrk` on my computer:

```bash
wrk -t16 -c40 -d30s http://127.0.0.1:4321/index.html

Running 30s test @ http://127.0.0.1:4321/index.html
  16 threads and 40 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.39ms  299.04us   8.09ms   74.38%
    Req/Sec   825.91     41.65     1.42k    67.23%
  394724 requests in 30.02s, 1.06GB read
  Socket errors: connect 0, read 394715, write 0, timeout 0
Requests/sec:  13148.46
Transfer/sec:     36.26MB
```

Compare to existing HTTP server frameworks:

flask with `gunicorn` and `gevent`
```bash
gunicorn -w 16 -k gevent -b 127.0.0.1:4321 flask_server:app

Running 30s test @ http://127.0.0.1:4321/index.html
  16 threads and 40 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     4.53ms    1.69ms 207.32ms   98.67%
    Req/Sec   317.75    115.83   505.00     59.74%
  106099 requests in 30.04s, 318.83MB read
  Socket errors: connect 0, read 0, write 0, timeout 22
Requests/sec:   3531.88
Transfer/sec:     10.61MB
```

python http package
```bash
python3 simple_http_server.py

Running 30s test @ http://127.0.0.1:4321/index.html
  16 threads and 40 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    27.21ms   78.98ms   1.77s    97.91%
    Req/Sec    53.27     27.50   121.00     73.50%
  9483 requests in 30.04s, 27.26MB read
  Socket errors: connect 0, read 0, write 0, timeout 5
Requests/sec:    315.67
Transfer/sec:      0.91MB
```

node + express
```bash
node express_server.js

Running 30s test @ http://127.0.0.1:4321/index.html
  16 threads and 40 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    10.54ms   13.85ms  79.75ms   85.49%
    Req/Sec   316.15    100.35   610.00     65.85%
  151218 requests in 30.03s, 453.28MB read
Requests/sec:   5035.55
Transfer/sec:     15.09MB
```

FastAPI + uvicorn
```bash
Running 30s test @ http://127.0.0.1:4321/index.html
  16 threads and 40 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    54.57ms    5.09ms  79.94ms   55.23%
    Req/Sec    36.64      6.45    50.00     75.54%
  17588 requests in 30.03s, 51.52MB read
Requests/sec:    585.67
Transfer/sec:      1.72MB
```

Summary Table

| Framework                | Requests/sec | Transfer/sec | Avg Latency | Max Latency | Total Requests | Total Data Read |
|--------------------------|--------------|--------------|-------------|-------------|----------------|-----------------|
| My C++ HTTP Server       | 13148.46     | 36.26MB      | 2.39ms      | 8.09ms      | 394724         | 1.06GB          |
| Flask with `gunicorn` + `gevent` | 3531.88  | 10.61MB      | 4.53ms      | 207.32ms    | 106099         | 318.83MB        |
| Python HTTP Package      | 315.67       | 0.91MB       | 27.21ms     | 1.77s       | 9483           | 27.26MB         |
| Node + Express           | 5035.55      | 15.09MB      | 10.54ms     | 79.75ms     | 151218         | 453.28MB        |
| FastAPI + `uvicorn`        | 585.67       | 1.72MB       | 54.57ms     | 79.94ms     | 17588          | 51.52MB         |

By no means is this a rigorous benchmark, but it gives a rough idea of the performance of my server compared to other frameworks.

TODO:
- [x] refactor into separate threadpool
- [x] profile the code and optimize
- [x] i feel like the way im working with strings is too slow...
