import requests
import concurrent.futures
import time
import subprocess

def send_request(server_url):
    try:
        response = requests.get(server_url)
        if response.status_code == 200:
            return True
        else:
            return False
    except requests.RequestException as e:
        print(f"Request failed: {e}")
        return False

def stress_test(server_url, num_threads, num_requests_per_thread):
    successful_requests = 0

    with concurrent.futures.ThreadPoolExecutor(max_workers=num_threads) as executor:
        futures = [executor.submit(send_request, server_url) for _ in range(num_threads * num_requests_per_thread)]
        for future in concurrent.futures.as_completed(futures):
            if future.result():
                successful_requests += 1

    return successful_requests

if __name__ == "__main__":
    server_process = subprocess.Popen(["./test"])

    time.sleep(2)

    server_url = "http://127.0.0.1:4321/index.html"
    num_threads = 16
    num_requests_per_thread = 100

    start_time = time.time()
    successful_requests = stress_test(server_url, num_threads, num_requests_per_thread)
    end_time = time.time()

    elapsed_time = end_time - start_time
    throughput = successful_requests / elapsed_time

    print("*" * 20)
    print(f"Total successful requests: {successful_requests}")
    print(f"Elapsed time: {elapsed_time:.2f} seconds")
    print(f"Throughput: {throughput:.2f} requests/second")
    print("*" * 20)

    server_process.terminate()
    server_process.wait()