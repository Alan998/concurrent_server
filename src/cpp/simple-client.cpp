// Launch n threads to connect to specified host:port
#include <netdb.h> // getaddrinfo
#include <sys/socket.h>
#include <unistd.h> // close

#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <span>
#include <string>
#include <thread>
#include <vector>

constexpr int BUFF_SIZE{ 1024 };

void
perror_die(const std::string_view msg)
{
	std::cerr << msg.data() << std::endl;
	std::exit(EXIT_FAILURE);
}

void
print_help()
{
	std::cout << "Usage: ./simple-client [-n=] [-port=] [-host=]\n";
	std::cout << "-n: launch n connection clients\n";
	std::cout << "-port: port to connect to server\n";
	std::cout << "-host: host name\n";
	std::cout << "-h, --help: print usage" << std::endl;
}

bool
check_end_connection(const std::span<char>& recv_data, const int recv_len)
{
	static int count{ 0 };
	for (int i{ 0 }; i < recv_len; ++i) {
		const char ch{ recv_data[i] };
		if (ch == '1') {
			if (++count == 4)
				return true;
		} else
			count = 0;
	}
	return false;
}

// get data sent from server
// when server sends back 4 consecutive 1's, client should close the socket
void
get_recv_data(std::string_view client_name, const int sockfd)
{
	std::array<char, BUFF_SIZE> buf;
	while (true) {
		const int recv_len = recv(sockfd, buf.data(), buf.size(), 0);
		// create a std::span
		std::span<char, BUFF_SIZE> recv_data{
			buf.data(), static_cast<unsigned long>(recv_len)
		};
		printf("%s received %.*s\n",
			   client_name.data(),
			   recv_len,
			   recv_data.data());
		if (check_end_connection(recv_data, recv_len))
			break;
	}
	printf("%s disconnecting...\n", client_name.data());
	close(sockfd);
}

void
send_data(const int							 sockfd,
		  const std::array<char, BUFF_SIZE>& buf,
		  const short						 data_len,
		  std::chrono::duration<float>		 sleep_time,
		  std::string_view					 client_name)
{
	printf("%s sending %s\n", client_name.data(), buf.data());
	if (send(sockfd, buf.data(), data_len, 0) < 0) {
		close(sockfd);
		perror_die("Error sending data");
	}
	std::this_thread::sleep_for(sleep_time);
}

// create a single socket connection to host:port
void
make_new_connection(std::string_view host,
					std::string_view port,
					std::string_view client_name)
{
	addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family	  = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* result;
	getaddrinfo(host.data(), port.data(), &hints, &result);

	// create socket
	auto sockfd =
		socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sockfd < 0) {
		perror_die("ERROR opening socket");
	}

	// connect to server
	if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
		close(sockfd);
		perror_die("ERROR connecting to server");
	}

	// wait for server to send start symbol '*'
	std::array<char, BUFF_SIZE> buf;
	const short recv_len = recv(sockfd, buf.data(), buf.size(), 0);
	if (recv_len < 0 || buf[0] != '*') {
		close(sockfd);
		perror_die("recv error");
	}
	// spawn a thread to receive server's data
	std::thread recv_thread(get_recv_data, client_name, sockfd);

	printf("%s connected...\n", client_name.data());

	// send data to server
	buf = { "^abc$de^abte$f" };
	short						 buf_data_len{ 14 };
	std::chrono::duration<float> sleep_time{ std::chrono::seconds(1) };
	send_data(sockfd, buf, buf_data_len, sleep_time, client_name);

	buf			 = { "xyz^123" };
	buf_data_len = 7;
	send_data(sockfd, buf, buf_data_len, sleep_time, client_name);

	buf			 = { "25$^ab0000$abab" };
	buf_data_len = 15;
	sleep_time	 = std::chrono::milliseconds(200);
	send_data(sockfd, buf, buf_data_len, sleep_time, client_name);

	recv_thread.join();
}

int
main(int argc, char* argv[])
{
	// setup arguments
	short		n_connection{ 1 };
	std::string port{ "8080" };
	std::string host{ "localhost" };

	// convert argv to vector of strings
	std::vector<std::string> arg_list(argv + 1, argv + argc);
	for (const auto& arg : arg_list) {
		if (arg.find("-n=") != std::string::npos) {
			n_connection = std::stoi(arg.substr(3));
		} else if (arg.find("-port=") != std::string::npos) {
			port = arg.substr(6);
		} else if (arg.find("-host=") != std::string::npos) {
			host = arg.substr(6);
		} else if (arg.find("-h") != std::string::npos || arg == "--help") {
			print_help();
		} else {
			print_help();
			perror_die("Invalid argument");
		}
	}

	// launch n connections
	auto					 start_time{ std::chrono::steady_clock::now() };
	std::vector<std::thread> threads;
	for (short i{}; i < n_connection; ++i) {
		std::string client_name = "client_" + std::to_string(i);
		threads.emplace_back(make_new_connection, host, port, client_name);
	}

	// wait for each thread to end
	for (auto& th : threads)
		th.join();

	auto end_time{ std::chrono::steady_clock::now() };
	auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
		end_time - start_time);
	std::cout << "Elapsed time: " << elapsed_time << " milliseconds"
			  << std::endl;
	return 0;
}
