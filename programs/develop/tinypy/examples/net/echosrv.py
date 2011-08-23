import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('192.168.1.101', 5000))
s.listen(1)
conn, addr = s.accept()
print("Connected by ", addr)
while True:
    data = conn.recv(1024)
    if not data:
        break
    print(data)
    conn.send(data)
conn.close()
