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
int computetriangle( char *buffer, int rows)
{
  int n, k, prntsz;
  long value; // Use long as numbers can be large
  char *bufptr = buffer;
  
  for( n =0; n < rows; n++) {
     value = 1;
     for ( k =0; k <= n; k++) {
       prntsz = sprintf(bufptr, "%ld ", value);
       value = value * (n - k) / (k + 1);
       bufptr += prntsz;
     }
     sprintf(bufptr, "\n");
     bufptr++;
  }
  return bufptr - buffer;
}

// Handle one client request
void handleclient( int sockchld) {
  char buffer[ 256] = {0};
  char bufferout[ 1024];
  int result, number;


  result = read( sockchld, buffer, sizeof( buffer));
  if ( result <= 0) {
    // Complain on error, but leave server running
    printf( "Read returned error or empty  data!\n");
    close( sockchld);
    return;
  }

  number = atoi( buffer);
  if ( number <= 0 ) {
    result = sprintf( bufferout, "Expecting to receive a number! Received: %s\n", buffer);
  } else {
    result = computetriangle( bufferout, number);
  }
    
  write( sockchld, bufferout, result);

  close( sockchld);
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
    printf("Error %d opening socketd!\n", errno);
    exit(1);
  }

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(port);
  ipaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  
  if ( bind( socklisten, (struct sockaddr *)&ipaddr, sizeof(struct sockaddr_in)) < 0) {
    printf("Error %d binding socketd!\n", errno);
    exit(1);
  }
  
  if ( listen( socklisten, 100) < 0 ) {
    printf("Error %d setting socket to listen!\n", errno);
    exit(1);
  }

  // Make it a daemon by detaching and setting a new session id
  procid = fork();
  if (procid < 0) {
    printf( "Fork failed!\n");
    exit(1);
  }
  
  if (procid > 0) {
    close( socklisten);
    printf( "Service started PID=%d\n", procid);
    exit(0);
  }

  sessid = setsid();
  if(sessid < 0) {
    printf( "Unable to create new daemon session\n");
    exit(1);
  }


  // Wait for incoming connections and do the useful work
  while ( 1) {
    while ( waitpid(-1, NULL, WNOHANG) > 0)
      ; // Clean child zombies
    
    sockchld = accept( socklisten, NULL, NULL);
    if ( sockchld < 0 ) {
      printf("Accept returned error %d!\n", errno);
      exit(1);
    }

    procid = fork();
    if (procid < 0) {
      printf( "Forking of client handler process failed!\n");
      exit(1);
    }
    
    if (procid > 0) {
      // Go to wait for next client connection
      close( sockchld);
    } else {
      handleclient( sockchld);
      exit( 0);
    }
  }
}
