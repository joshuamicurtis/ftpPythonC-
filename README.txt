To run the ftp server:
1. Put ftserver.cpp in a directory on a server, with a few files in the directory.
2. From the command line compile the server by running: g++ -o ftserver ftserver.c
3. To start the server program, in the command line enter: ftserver xxxxx
4. xxxxx is the port number the server is listening on.

To run the client:
1. Put ftclient.py in a separate computer from ftpserver.cpp
2. From the command line run: python ftclient.py hostname xxxxx -g/-l [fileName] yyyyy
3. hostname is the hostname of the server with ftserver.cpp in it
4. xxxxx is the port number of that ftserver is listening on.
5. Select either -g or -l.
6. If you select -g you must also include a file name you would would like to recieve from the server
7. If you select -l the server will send you a list of files in the directory that ftserver is in.
8. yyyyy is the port number the client program will receive the data from the server on.