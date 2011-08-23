import kolibri

socket = kolibri.socket # Alias for socket module

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind('10.0.2.15', 6000)
if s.connect('192.168.1.101', 5000):
    print("cp2, socket is", s.socket)
    if not s.send("<html></html>"):
        print("Sending failed")
    else:
        data = s.recv(1024)
        print(data)
    s.close()
else:
    print("Connection failed, socket is", s.socket)
print("OK")
