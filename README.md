# Web-Server

This is a simple web server in C/C++ using TCP sockets. My server is able to handle connections from multiple clients (maximum of 10) using port number 30000 as the listening port, accept and parse HTTP requests, retrieve files from the server’s file system, and build and send HTTP response messages.

HTTP Requests - This server is be able to handle two types of requests: GET, and the conditional GET. 

Connection Types - This server only deals with HTTP version 1.1, and thus uses persistent connections. The server detects if the client (web browser) closes its end of the connection and use that as a signal that the server should close its end also. In addition, the server should timeout and close the connection if there are no requests on the connection after 10 seconds.

HTTP Options - This server is able to handle the following request options: Connection (in v1.1, the default is keep-alive), and If-Modified-Since. This server handles the following request options: Host, User-agent, Accept, and Accept-Language.

When sending responses, the server includes the following options (where appropriate): Date, Server, Last-Modified, Content-Length, Content-Type, and Connection.

Content Types - This server is able to handle a number of different file types. Thus, it is able to determine the type of file from the extension. The server uses the following content types for the indicated file extensions:

text/html      .html
image/gif      .gif
image/jpeg     .jpeg
image/jpeg     .jpg
text/plain     all others

Root Directory - A “root” web directory was created within the folder in which your source file is stored and use that directory for extracting files. The path name stored in the URL in the GET or POST request begins with the name of the root directory. In addition, if the URL ends with a slash, then it is required to append the file name index.html to the end of the URL.

Security - If you are not careful when running your web server, you will give the world access to all of your files on the system. Therefore, you must make sure the URL in a request does not contain the substring “../”. If it does, then return an error message indicating the file could not be found on the server.

Errors - The only error responses handled are the following:

200    OK
304    Not Modified
400    Bad Request
404    Not Found

The server also returns a simple HTML document in the message body indicating the particular error for all errors other than 304. Note, the returned document is not extracted from the file system, but instead hard coded within the program.

File Attributes - Access is needed to the file attributes for the various files retrieved by your server. The stat() function is used and provided in the Standard C library in the header file sys/stat.h.

Parsing and Printing Dates - The Standard C library provides a number of functions for working with dates. The following functions were useful:

strftime()    strptime()    gmtime()

String Processing - This program involves a large amount of string processing. As you know, working with strings in C can be challenging. The server utilizes  a split function that can be used to split a string into one or more parts based on a given delimiter.

Program Structure - Server design is well structured program and uses a Makefile to build the program. The program is well commented. The executable is named httpd. When the program is built, all that is required should be to enter 'make httpd' and when the program is ran, you should be able to enter 'httpd' and assume the use of socket number 30000.
