#! /usr/bin/env python
# Program Name: ftserver
# Author: Joshua Curtis
# Date: November, 29 2015
# Description: 
# - Server for file transfer program
import socket 
import sys

def controlConnect():
    HOST = sys.argv[1]
    TCP_PORT = int(float(sys.argv[2]))
    BUFFER_SIZE = 1024
    data = sys.argv[3]
    DATA_PORT = 0
#Validate input from command line
    if data == "-g":
        print "requesting file"
        if len(sys.argv) != 6:
	        print "Incorrect number of arguments with -g option"
	        sys.exit()
        else:	    
	        data += " "
	        data += sys.argv[4]
	        data += " "
	        data += sys.argv[5]
	        data += " "
	        data += socket.gethostname()
	        DATA_PORT = int(float(sys.argv[5]))
    elif data == "-l":
        print "requesting directory list"
        if len(sys.argv) != 5:
	        print "Incorrect number of arguments with -l option"
	        sys.exit()
        else:
	        data += " "
	        data += sys.argv[4]
	        data += " "
	        data += socket.gethostname()
	        data += " 1"
	        DATA_PORT = int(float(sys.argv[4])) 
    else:
        print ("Invalid option, must be -g or -l")
        sys.exit()
#Create a TCP socket  
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect to server and send data
    s.connect((HOST, TCP_PORT))
# Send data
    s.send(data)

# Open connection to recieve server response
    DATA_HOST = socket.gethostname()
#Create a TCP socket and listen for server
    d = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    d.bind((DATA_HOST, DATA_PORT))
    d.listen(1)		
 
    conn, addr = d.accept()
#    print 'Connection address:', addr
    text = ""
    while 1:
        response = conn.recv(BUFFER_SIZE)
        if not response: break
#        print "received data:", response
        text += response
    conn.close()
    d.close();
#    received = s.recv(BUFFER_SIZE)
    s.close()
    if sys.argv[3] == "-l":
	    print "receiving directory structure from", sys.argv[1], sys.argv[4]
	    print text
    if sys.argv[3] == "-g" and text != "FILE NOT FOUND":
	    print "receiving", sys.argv[4], "from", sys.argv[1], sys.argv[5]
	    print "file transfer complete"
	    newFile = open(sys.argv[4], 'w')
	    newFile.write(text)
    if text == "FILE NOT FOUND":
	    print sys.argv[1], sys.argv[5], "says", text
def main():
    controlConnect()
	
main()