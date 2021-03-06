/*
  A very simple daemon tha generates a pascal triangle in response to a number
  sent over a TCP/IP connection.

  Copyright (C) 2017 Roque Luis Scheer <roqscheer@gmail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.  
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>


// Generates a pascal triangle in printable format, including newline characters
int computetriangle( char *buffer, const int bufsz, const int rows)
{
  int n, k, prntsz,buffersz;
  int resultsz = 0;
  long value; // Use long as numbers can be large
  char *bufptr = buffer;
  
  buffersz = bufsz - 2; //Leave room for final newline and null char
  for( n = 0; n < rows && resultsz < buffersz; n++) {
     value = 1;
     for ( k =0; k <= n; k++) {
       prntsz = snprintf(bufptr, buffersz - resultsz, "%ld ", value);
       value = value * (n - k) / (k + 1);
       bufptr += prntsz;
       resultsz = bufptr - buffer;
     }
     snprintf(bufptr, 2, "\n");
     bufptr++;
     resultsz++;
  }
  return resultsz > bufsz ? -1 : resultsz;
}

// Handle one client request
void handleclient( int sockchld) {
  char buffer[ 256] = {0};  // Clean it to ensure will have trailing null char
  char bufferout[ 4096];    // To find out the exact triangle size, we need to generate it,
                            // so use a large buffer just to be safe
  int result, number, triangsz;


  while ( ( result = read( sockchld, buffer, sizeof( buffer) - 1)) > 0 ) {
    number = atoi( buffer);
    if ( number <= 0 ) {
      triangsz = sprintf( bufferout, "Expecting to receive number of lines! Received: %s\n", buffer);
    } else {
      triangsz = computetriangle( bufferout, sizeof(bufferout), number);
    }
    
    if (triangsz < 0) {
      triangsz = sprintf( bufferout, "Resulting triangle exceeded %d chars\n", 
                         (int) sizeof(bufferout));
    }
        
    write( sockchld, bufferout, triangsz);
    memset( buffer, 0x00, sizeof(buffer));
  }

  close( sockchld);
  if ( result < 0) {
    // Abort working process with error
    fprintf( stderr, "Read returned error %d!\n", errno);
    exit( -1);
  }

  exit( 0);
}

int main(int argc, char* argv[])
{
  pid_t procid = 0;
  pid_t sessid = 0;
  int socklisten = 0;
  int sockchld = 0;
  int port = 0;
  struct sockaddr_in ipaddr;

  if ( argc > 2 ||
       (argc == 2 && (port = atoi(argv[1])) == 0 ) ) { // Any argv[1] <> from a valid port prints help
    printf( "Compute Pascal triangles service\n");
    printf( "usage: %s [port]  # Port defaults to 55555\n", argv[0]);
    exit( 0);
  }

  if ( port == 0) {
    port = 55555;
  }

  // Do most of the sock setup to validate network before going daemon
  socklisten = socket( AF_INET, SOCK_STREAM, 0);
  if ( socklisten < 0) {
    fprintf( stderr, "Error %d opening socketd!\n", errno);
    exit(1);
  }

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(port);
  ipaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  
  if ( bind( socklisten, (struct sockaddr *)&ipaddr, sizeof(struct sockaddr_in)) < 0) {
    fprintf( stderr, "Error %d binding socketd!\n", errno);
    exit(1);
  }
  
  if ( listen( socklisten, 100) < 0 ) {
    fprintf( stderr, "Error %d setting socket to listen!\n", errno);
    exit(1);
  }

  // Make it a daemon by detaching and setting a new session id
  procid = fork();
  if (procid < 0) {
    fprintf( stderr, "Fork failed!\n");
    exit(1);
  }
  
  if (procid > 0) {
    close( socklisten);
    printf( "Service started PID=%d\n", procid);
    exit(0);
  }

  sessid = setsid();
  if(sessid < 0) {
    fprintf( stderr, "Unable to create new daemon session\n");
    exit(1);
  }


  // Wait for incoming connections and do the useful work
  while ( 1) {
    while ( waitpid(-1, NULL, WNOHANG) > 0)
      ; // Clean child zombies
    
    sockchld = accept( socklisten, NULL, NULL);
    if ( sockchld < 0 ) {
      fprintf( stderr, "Accept returned error %d!\n", errno);
      exit(1);
    }

    procid = fork();
    if (procid < 0) {
      fprintf( stderr, "Forking of client handler process failed!\n");
      exit(1);
    }
    
    if (procid > 0) {
      // Go to wait for next client connection
      close( sockchld);
    } else {
      handleclient( sockchld);  // Does not return
    }
  }
}
