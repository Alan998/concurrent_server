// Sequential server
#include "utils.hpp"
#include <array>
#include <iostream>
#include <unistd.h>

enum class Process_State
{
	WAIT_FOR_MSG,
	IN_MSG
};

void
serve_connection(const int sockfd)
{
	// Clients attempting to connect and send data will succeed even before the
	// connection is accept()-ed by the server. Therefore, to better simulate
	// blocking of other clients while one is being served, do this "ack" from
	// the server which the client expects to see before proceeding.
	if (send(sockfd, "*", 1, 0) < 1) {
		perror_die("send error");
	}

	Process_State state{ Process_State::WAIT_FOR_MSG };
	while (true) {
		std::array<uint8_t, 1024> buf;
		const int				  len = recv(sockfd, buf.data(), buf.size(), 0);
		if (len < 0) {
			perror_die("recv error");
		} else if (len == 0) {
			break;
		}

		for (int i = 0; i < len; ++i) {
			switch (state) {
				case Process_State::WAIT_FOR_MSG:
					if (buf[i] == '^') {
						state = Process_State::IN_MSG;
					}
					break;
				case Process_State::IN_MSG:
					if (buf[i] == '$') {
						state = Process_State::WAIT_FOR_MSG;
					} else {
						// add 1 to each character received and send it back
						buf[i] += 1;
						if (send(sockfd, &buf[i], 1, 0) < 1) {
							perror("send error");
							close(sockfd);
							return;
						}
					}
					break;
				default:
					perror_die("Unknown process state");
			}
		}
	}
	// close connection
	close(sockfd);
}

int
main(int argc, char* argv[])
{
	int port_num{ 8080 };
	if (argc >= 2) {
		port_num = std::atoi(argv[1]);
	}
	std::cout << "Press <Ctrl + c> to quit\n";
	std::cout << "Serving on port: " << port_num << std::endl;

	auto sockfd = listen_inet_socket(port_num);
	while (true) {
		sockaddr_in peer_addr;
		socklen_t	peer_addr_len{ sizeof(peer_addr) };

		// accept connection
		int new_sockfd = accept(
			sockfd, reinterpret_cast<sockaddr*>(&peer_addr), &peer_addr_len);
		if (new_sockfd < 0) {
			perror_die("Error on accept");
		}
		report_peer_connected(&peer_addr, peer_addr_len);
		serve_connection(new_sockfd);
		std::cout << "peer done\n";
	}
	return 0;
}
