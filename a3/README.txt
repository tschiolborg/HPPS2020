README 

The program "file_server.py" runs a static file server that serves files from the directory "Webroot". 
Place the directory on a given path, and change the parameter "path_root" accordingly

"file_server.py" can be called with 0 input arguments, 2 input arguments, and 3 input arguments (not counting the program as an argument)
Input arguments are as follows:
1st: Bind-IP
2nd: process-PORT
3rd: update index.html: True/False
Example:
"python file_server.py all 8080 True"

When the server is running, requests can be made to the server using the server's local IP (or localhost if run internally)
Example entry in browser:
"192.168.14.167:8080"

If the server should be open for requests for all devices on a LAN, also add a firewall rule.
Example on ubuntu:
#sudo ufw allow from 192.168.0.0/20 to any port 8080

"test_client.py" is a test script.

