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

TODO:
- [x] refactor into separate threadpool
- [x] profile the code and optimize
- [x] i feel like the way im working with strings is too slow...
