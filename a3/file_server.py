import socket
import re
import subprocess
import os
from threading import Thread, active_count
import sys

path_root = os.path.expanduser("~/Desktop/Webroot")

#INPUTS:
#argv[1] = BIND-IP
#argv[2] = process-PORT
#argv[3] = Update index T/F

#bind-host input
if len(sys.argv) > 2: #IP and PORT must be set together
    host_str = r"^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}|localhost)$" #roughly valid IPv4
    host_regex = re.compile(host_str)
    if host_regex.search(sys.argv[1]):
        HOST = sys.argv[1]
    elif sys.argv[1] == 'all':
        HOST = '' #bind on all IP's
    else:
        raise ValueError('1st argument must be valid IPv4, localhost, or all')
else:
    HOST = 'localhost'

if len(sys.argv) > 2: #IP and PORT must be set together
    try:
        PORT = int(sys.argv[2])
        if not (PORT in range(2**16)):
            raise ValueError('2nd argument must be valid PORT')
    except ValueError:
        print('2nd argument must be valid PORT')
else:#default
    PORT = 8080

#update input
if len(sys.argv) == 4:
    if sys.argv[3].lower() == 'false':
        update_index_file = False # set true only for Webroot server
    elif sys.argv[3].lower() == 'true':
        update_index_file = True
    else:
        raise ValueError('3rd argument must be either True or False')
else:#default
    update_index_file = False


"""Parses the HEADER, given input data expected to be header"""
class Parser_rfc_2616():
    #defining parts of regexes
    method_str = r"(?:OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT)"
    path_str = r"/\S*"
    http11_str = r"HTTP/1\.1"
    line_str = r"\r\n"
    host_init_str = r"Host: "
    if HOST == '': #all IP's allowed
        host_str = f".+:{PORT}"
    else: #only specific IP
        host_str = f"{HOST}:{PORT}"
    head_opt_str = r"(?:.+:.+\r\n)*"
    body_str = r"[^\r]*"

    #ordering of regexes
    str_order = [r"^",method_str, r" ", path_str, r" ", http11_str, line_str,
                head_opt_str, host_init_str, host_str, line_str, head_opt_str,
                line_str, body_str, r"$"]

    def __init__(self, data):
        self.data = data.decode('utf-8') #decoded data

    def check_valid(self):
        """Checking if data is a valid HTTP/1.1 request.
        Request must also be minimal"""
        check_regex = re.compile("".join(self.str_order))
        if check_regex.search(self.data):
            return True
        else:
            return False

    def get_method(self):
        """Return method of request"""
        method_regex = re.compile(r"^(" + self.method_str + r") ")
        method = method_regex.findall(self.data)
        if method:
            return method[0]
        else:
            return None

    def get_path(self):
        """Return path of request"""
        path_regex = re.compile("".join(self.str_order[:3]) + r"(" + self.path_str + r") ")
        path = path_regex.findall(self.data)
        if path:
            return path[0]
        else:
            return None


def set_flags(PORT_path):
    """flags for linux tree"""
    #make sure root is correctly named
    if HOST in ['127.0.0.1', 'localhost']:
        host_ip = HOST #root folder is pathed to localhost
    else: #not localhost
        hostname = socket.gethostname()
        host_ip = socket.gethostbyname(hostname) #root folder is pathed to local address
    flags = f"-C -H http://{host_ip}:{PORT_path} --noreport" 
    return flags


def update_index():
    """Find tree, update index.html"""
    try:
        #flags for linux tree
        flags = set_flags(PORT)

        #get tree structure of web directory
        process = subprocess.Popen([f"tree {path_root} {flags}"],stdout=subprocess.PIPE,stderr=subprocess.PIPE, shell=True)
        out, _ = process.communicate() #out is a bytestring
        process.terminate()

        #write to index.html (update)
        with open(path_root+"/index.html","wb") as f:
            f.write(out)

        return True
    except:
        return False


def get_index(path):
    """Return index.html is exists"""
    try: # try to return index.html
        with open(path_root+path+"/index.html","rb") as f:
            index_html = f.read()
            return index_html, len(index_html)
    except:
        # if index.html does not exists: try to write tree
        try:
            #flags for linux tree
            if path[-1] == '/': #html tree adds "/" in certain cases
                flags = set_flags(str(PORT)+path[:-1])
            else:
                flags = set_flags(str(PORT)+path)

            #get tree structure of web directory
            process = subprocess.Popen([f"tree {path_root+path} {flags}"],stdout=subprocess.PIPE,stderr=subprocess.PIPE, shell=True)
            out, _ = process.communicate() #out is a bytestring
            process.terminate()
            
            return out, len(out)
        except:
            return None, 0

def content_type(path):
    try:
        # get MIME type of file
        process = subprocess.Popen(f"file --mime-type -b {path}",stdout=subprocess.PIPE,stderr=subprocess.PIPE, shell=True)
        out, _ = process.communicate() #out is a bytestring
        process.terminate()
        return out.decode()[:-1] # remove last element (\n)
    except:
        return None

def give_response(request):
    """Evaulates parsed request and return proper response"""
    http_ver = "HTTP/1.1 "
    line_shift = "\r\n"
    c_close = "Connection: close"
    end_str = line_shift + c_close + line_shift*2

    code_200 = lambda length, ct: (http_ver + "200 OK" + line_shift + f"Content-Type: {ct}" + line_shift +
        f"Content-Length: {length}" + end_str).encode()
    code_400 = (http_ver + "400 Bad Request" + end_str).encode()
    code_404 = (http_ver + "404 Not Found" + end_str).encode()
    code_405 = (http_ver + "405 Method Not Allowed" + line_shift + "Allow: GET, HEAD" + end_str).encode()
    code_500 = (http_ver + "500 Internal Server Error" + end_str).encode()
    
    
    #instanciate parser with request
    parser = Parser_rfc_2616(request)

    #possibly 400 bad request
    if not parser.check_valid():
        return code_400
    
    #get parsed info
    method = parser.get_method()
    path = parser.get_path()
    
    #possibly 405 Method not allowed
    if method not in ["HEAD", "GET"]:
        return code_405
    
    #404 code when possibly trying to access parent directory
    if '..' in path.split('/'):
        return code_404

    #possibly pathed to folder -> respond with index.html
    if os.path.isdir(path_root+path):
        body, body_length = get_index(path)
        if body is None:
            return code_500
        
        response = code_200(body_length, 'text/html')
        
        if method == "GET":
            response += body
        return response
    #pathed to file
    elif os.path.isfile(path_root+path):
        #file is either there or not, or another exception occured when trying to open
        try:
            with open(path_root+path,"rb") as f:
                body = f.read()
            
            body_length = len(body)

            # set Content-Type
            ct_type = content_type(path_root+path)
            if ct_type:
                response = code_200(body_length, ct_type)
            else:
                response = code_200(body_length, 'application/octet-stream')
            
            if method == "GET":
                response += body 

        except FileNotFoundError:
            response = code_404
        except:
            response = code_500
        finally:
            return response
    else:
        return code_404


def run_new_client(conn, addr):
    """called when a new connection is made"""
    print('Connection established with: ', addr)
    
    while True:
        request = conn.recv(1024) #byte-data
        print("Received request:\n" + request.decode("utf-8"))
        
        response = give_response(request)
        
        conn.sendall(response)
        print("A response has been sent")

        conn.close()
        print("Connection closed: ", addr)
        break

#update index.html at start
if update_index_file:
    if not update_index(): 
        print('error when updating index.html')

#run server
with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST,PORT))
    s.listen()

    while True:
        conn, addr = s.accept() #connect-socket, (host-IP,host-port)
        Thread(target=run_new_client, args=(conn,addr)).start()
        print(f"total connections: {active_count()-1}")





    