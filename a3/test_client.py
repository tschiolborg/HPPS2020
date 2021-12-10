import socket
import os

HOST = '127.0.0.1' #IP of SERVER
PORT = 8080 #port of SERVER
#client port is implicitly allocated (random)

path_root = os.path.expanduser("~/Desktop/Webroot")

num_tests = 0
num_fails = 0



def test_req(req_string, test_msg, res_expected):
    """Send request (bytestring) to server and give output"""
    result = ''

    res = b''
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.connect((HOST,PORT))
        s.sendall(req_string)
        
        while True:
            data = s.recv(1024) #store answer, max 1024 bytes
            if not data:
                break
            res += data

        if res == res_expected:
            result = ' Success   '
        else:
            result = ' Fail      '
            global num_fails
            num_fails += 1 
    global num_tests
    num_tests += 1
    print(result,test_msg)



with open(path_root+"/index.html", 'rb') as f:
    index_html_1 = f.read()

with open("index_site1.html", 'rb') as f:
    index_html_2 = f.read()

with open(path_root+"/site3/site3_1/site3_1_1/Assignment2.pdf", 'rb') as f:
    pdf_file = f.read()

with open(path_root+"/site3/site3_1/site3_1_1/hex_to_binary.png", 'rb') as f:
    png_file = f.read()

with open(path_root+"/site2/some_video.mov", 'rb') as f:
    mov_file = f.read()

with open(path_root+"/site2/some_audio.m4a", 'rb') as f:
    m4a_file = f.read()


### Basic tests

mes = '---Test of method HEAD on start folder---'
req = (f'HEAD / HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(index_html_1)}\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET on start folder---'
req = (f'GET / HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(index_html_1)}\r\nConnection: close\r\n\r\n').encode() + index_html_1  
test_req(req, mes, res)

mes = '---Test of method HEAD on file ---'
req = (f'HEAD /site1/doc1.txt HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET on file ---'
req = (f'GET /site1/doc1.txt HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\nConnection: close\r\n\r\n').encode() + b'abc\n'
test_req(req, mes, res)


### Advanced positive tests

mes = '---Test of method GET on folder without index.html ---'
req = (f'GET /site1/ HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(index_html_2)}\r\nConnection: close\r\n\r\n').encode() + index_html_2
test_req(req, mes, res)

mes = '---Test of method GET with extension ---'
req = (f'GET /site1/doc1.txt HTTP/1.1\r\nHost: {HOST}:{PORT}\r\nAccept-Language: da-DK,da;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\nConnection: close\r\n\r\n').encode() + b'abc\n'
test_req(req, mes, res)

mes = '---Test of method GET with a body ---'
req = (f'GET /site1/doc1.txt HTTP/1.1\r\nHost: {HOST}:{PORT}\r\nContent-Type: text/plain\r\nContent-Length: 4\r\n\r\nabc').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\nConnection: close\r\n\r\n').encode() + b'abc\n'
test_req(req, mes, res)

mes = '---Test of method GET on pdf file ---'
req = (f'GET /site3/site3_1/site3_1_1/Assignment2.pdf HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: {len(pdf_file)}\r\nConnection: close\r\n\r\n').encode() + pdf_file  
test_req(req, mes, res)

mes = '---Test of method GET on png file ---'
req = (f'GET /site3/site3_1/site3_1_1/hex_to_binary.png HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: {len(png_file)}\r\nConnection: close\r\n\r\n').encode() + png_file  
test_req(req, mes, res)

mes = '---Test of method GET on mov file ---'
req = (f'GET /site2/some_video.mov HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: video/quicktime\r\nContent-Length: {len(mov_file)}\r\nConnection: close\r\n\r\n').encode() + mov_file  
test_req(req, mes, res)

mes = '---Test of method GET on audio file ---'
req = (f'GET /site2/some_audio.m4a HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 200 OK\r\nContent-Type: audio/x-m4a\r\nContent-Length: {len(m4a_file)}\r\nConnection: close\r\n\r\n').encode() + m4a_file  
test_req(req, mes, res)


### Tests with errors

mes = '---Test of method GET without Host in header ---'
req = (f'GET /site1/doc1.txt HTTP/1.1\r\n\r\n').encode()
res = (f'HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET with wrong http version ---'
req = (f'GET /site1/doc1.txt HTTP/1.0\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET with error in header ---'
req = (f'GET /site1/doc1.txt HTTP/1.1\r\nHost: {HOST}:{PORT}\r\nfahfuihwafijwfkj\r\n\r\n').encode()
res = (f'HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET with only one \\r\\n at end of header ---'
req = (f'GET /site1/doc1.txt HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n').encode()
res = (f'HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method different than GET and HEAD ---'
req = (f'PUT / HTTP/1.1\r\nHost: {HOST}:{PORT}\r\nContent-type: text/html\r\nContent-length: 16\r\n\r\n<p>New File</p>').encode()
res = (f'HTTP/1.1 405 Method Not Allowed\r\nAllow: GET, HEAD\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET on file that does not exists ---'
req = (f'GET /site1/file_that_does_not_exists.txt HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET on directory that does not exists ---'
req = (f'GET /dir_that_does_not_exists/ HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)

mes = '---Test of method GET where path leads to parent directory ---'
req = (f'GET /../ HTTP/1.1\r\nHost: {HOST}:{PORT}\r\n\r\n').encode()
res = (f'HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n').encode()
test_req(req, mes, res)


print(f'Number of success: {num_tests-num_fails}/{num_tests}')
print(f'Number of fails: {num_fails}')
