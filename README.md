This is a simple linux daemon to demonstrate the basics of how to implement services in Linux.

It listens to client requests over a TCP port on the localhost, and generates pascal triangles
(https://en.wikipedia.org/wiki/Pascal's_triangle) in text format (including newline characters),
sending them back to the client over the same connection.

Each client connection can handle multiple requests. The connection to the client is maintained until the client closes it.

To launch the service, use this command line: 

    $ ./pascaltri [port_number] 

If not informed, the port_number defaults to 55555. 

To test the service you can use telnet as the client application: 

    $ telnet localhost port_number 
    
Close the telnet connection gracefully by typing the escape character (typically '^]') and then the "close" telnet command.
    
