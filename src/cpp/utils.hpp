#pragma once

#include <netinet/in.h>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>

// Dies (exits with a failure status) after printing the current perror status
// prefixed with msg.
void
perror_die(const std::string_view msg);

// Reports a peer connection to stdout. sock_addr is the data populated by a
// successful accept() call.
void
report_peer_connected(sockaddr_in* sock_addr, const socklen_t sock_addr_len);

// Creates a bound and listening INET socket on the given port number. Returns
// the socket fd when successful; dies in case of errors.
int
listen_inet_socket(const int portnum);
