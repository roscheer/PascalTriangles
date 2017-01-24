This is a simple linux daemon to demonstrate the basics of how to implement services in Linux.

It listens to client requests over a TCP port on the localhost, and generates a pascal triangle
(https://en.wikipedia.org/wiki/Pascal's_triangle) in text format (including newline characters),
sending it back to the client over the same connection.

Each client connection is good for only one request, as the connection is closed after 
sending the response. To request a new triangle, the connection needs to be restablished.

To test the service, launch it and use the ncat command: 
    $ ncat localhost port_number 
    
If not informed when launching the service, the port_number defaults to 55555. Remeber to stop and restart ncat for each new request, as the server closes the connection after it replies.

Possible improvements:
    Continue to process requests until the client closes the connection.
    Print error messages to stderr.

