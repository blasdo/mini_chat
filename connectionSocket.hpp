#ifndef CONNECTION_SOCKET_HPP
# define CONNECTION_SOCKET_HPP
# include "sys/socket.h"
# include "netinet/in.h"
# include "netinet/tcp.h"
# include "poll.h"
#include <exception>
#include <sys/types.h>
# include <vector>
# include <string>
# define TIMEOUT 100


// ConnectionSocket is the class that abstract a whole TCP Connection
// There will one instance per open port.

//Message created via Connection Socket. Its creted when the ConectionSocket
// needs to comunicate with another module
//
class ConnectionSocket;
class NetMessage
{
	private:
		ConnectionSocket &sock;
		const int fd; //sender (fd)
	public:
		const std::string message; //message means all charecters that can be readed
		NetMessage(ConnectionSocket &sock, int fd, std::string message);
		int	response(std::string message); // sends message though connection socket
		~NetMessage(); //Delete message;


};

/*
 * A connection socket is the abstracion of "socket interface" of unix for
 * this project
 */
class ConnectionSocket
{
	private:
		std::vector<sockaddr_in> addrs;
		std::vector<pollfd> fds;
		std::vector<char*> names;
		pollfd *findpollfd(int fd);
		NetMessage *readClientMessage(int fd);

		// Removes ALL fd that are disconected, it's called
		// at the end of every "cycle"
	public:
		class PollException: public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return "poll(2) execution failed";
				}
		};
		ConnectionSocket(int port);
		// Preform a poll before doing anything.
		// Please, note that remves all "revent" if there are "hup" and prepare
		// These fd to remove with garbage collector.
		void		performPoll();
		void		garbageCollector();
		void		WaitNewConnection();
		ssize_t		sendMessage(int fd, std::string msg);
		ssize_t		sendBroadcastMessage(std::string message);
		NetMessage *getNextMessage();
};

#endif
