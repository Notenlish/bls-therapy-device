import socket

HOST = "0.0.0.0"
PORT = 8290

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((HOST, PORT))
sock.listen(1)

print(f"Server listening on port {PORT}...")

conn, addr = sock.accept()
print("Connected by", addr)

try:
    while True:
        data = conn.recv(1024)
        if not data:
            break
        msg = data.decode("utf-8").strip()
        print("From ESP32:", msg)

        # send a reply
        reply = f"Got: {msg}\n"
        conn.sendall(reply.encode("utf-8"))
finally:
    conn.close()
    sock.close()