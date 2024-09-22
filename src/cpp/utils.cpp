#include "utils.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h> // getnameinfo

constexpr int N_BACKLOG{ 64 };

void
perror_die(const std::string_view msg)
{
	std::cerr << msg.data() << std::endl;
	std::exit(EXIT_FAILURE);
}

void
report_peer_connected(sockaddr_in* sock_addr, const socklen_t sock_addr_len)
{
	std::array<char, NI_MAXHOST> host_buffer;
	std::array<char, NI_MAXSERV> port_buffer;
	if (getnameinfo(reinterpret_cast<sockaddr*>(sock_addr),
					sock_addr_len,
					host_buffer.data(),
					NI_MAXHOST,
					port_buffer.data(),
					NI_MAXSERV,
					0) == 0) {
		printf("peer (%s, %s) connected\n",
			   host_buffer.data(),
			   port_buffer.data());
	} else {
		printf("peer (unknonwn) connected\n");
	}
}

int
listen_inet_socket(const int port_num)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror_die("ERROR opening socket");
	}

	// This helps avoid spurious EADDRINUSE when the previous instance of this
	// server died.
	int opt{ 1 };
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror_die("setsockopt failed");
	}

	sockaddr_in serv_addr;
	std::memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family	  = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port		  = htons(port_num);

	if (bind(sockfd,
			 reinterpret_cast<sockaddr*>(&serv_addr),
			 sizeof(serv_addr)) < 0) {
		perror_die("ERROR on binding");
	}

	if (listen(sockfd, N_BACKLOG) < 0) {
		perror_die("ERROR on listen");
	}
	return sockfd;
}
