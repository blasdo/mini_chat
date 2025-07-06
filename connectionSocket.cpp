#include "connectionSocket.hpp"
#include <exception>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
# include <unistd.h>


NetMessage::NetMessage(ConnectionSocket &sock, int fd, std::string message): sock(sock), fd(fd), message(message)
{}

int NetMessage::response(std::string message)
{
	return (sock.sendMessage(fd, message));
}
NetMessage::~NetMessage()
{}
/*
 * Please note that fds[0] is the socket itself
 */


 /*
  * Verify if fd socket is part of the socket "fds" vector.
  */

  NetMessage *ConnectionSocket::readClientMessage(int fd)
  {
  	int			readret;
 	char		buffer[1024];
  	std::string message;

   do{
  		readret = read(fd, buffer, 1023);
    	buffer[readret] = '\0';
     message.append(buffer);
   }while(readret == 1023);
   return (new NetMessage(*this, fd, message));
  }


void	ConnectionSocket::garbageCollector()
{
	for (int i = 0; i < fds.size(); i++)
	{
		if (fds[i].revents == POLLHUP)
		{
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
		}
	}
}
pollfd *ConnectionSocket::findpollfd(int fd)
{
	if (fd == fds[0].fd)
		return NULL;
	for (int i = 0; i < fds.size(); i++)
	{
		if (fds[i].fd == fd)
			return &fds[i];
	}
	return NULL;
}

/*
 * Initialize a Conection Socket through "port(4)" port
 */
ConnectionSocket::ConnectionSocket(int port)
{
	sockaddr_in address;
	pollfd		sockfd;

	fds = std::vector<pollfd>();
	addrs = std::vector<sockaddr_in>();

	sockfd.fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	sockfd.events = POLLIN;
	sockfd.revents = 0;
	address.sin_port = htons(port);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd.fd, (sockaddr*) &address, (socklen_t )sizeof(sockaddr_in)))
		throw std::runtime_error("Bind error");
	if (listen(sockfd.fd, 10))
		throw std::runtime_error("Listen error");
	fds.push_back((sockfd));
	addrs.push_back(address);
}

void	ConnectionSocket::performPoll()
{
	if (poll(fds.data(), (int) fds.size(), (int) TIMEOUT) < 0)
		throw PollException();
	for (int i = 0; i < fds.size(); i++)
	{
		if (fds[i].revents & POLLHUP)
			fds[i].revents = POLLHUP;
	}
}

void ConnectionSocket::WaitNewConnection()
{
	pollfd 		pfd;
	sockaddr_in	addr;
	int			addr_size = sizeof(addr);

	pfd.events = POLLIN | POLLOUT | POLLHUP;
	pfd.revents = 0;
	pfd.fd = accept(fds[0].fd, (sockaddr*)&addr, (socklen_t*) &addr_size);
	if (pfd.fd < 0)
		return ;
	fds.push_back(pfd);
	addrs.push_back(addr);
}

ssize_t ConnectionSocket::sendMessage(int fd, std::string msg)
{
	int characters_printed = 0;
	pollfd *pfd;

	pfd = findpollfd(fd);
	if (pfd)
	{
		performPoll();
		if (pfd->revents & POLLOUT && !(pfd->revents & POLLHUP))
			return write(pfd->fd, msg.c_str(), msg.length());
		else
 			return -1;
	}
	return (-1);
}

ssize_t	ConnectionSocket::sendBroadcastMessage(std::string message)
{
	ssize_t total = 0;
	ssize_t	res = 0;
	for (int i = 1; i < fds.size(); i++)
	{
		res = sendMessage(fds[i].fd, message);
		if (res < 0)
			total = -1;
		else if (total != -1)
			total += res;
	}
	return total;

}

// for each client revise if there are anyting for read.
// Create a message if there are anyting for read
NetMessage *ConnectionSocket::getNextMessage()
{
	NetMessage *result;

	this->performPoll();
	for (int i = 1; i < fds.size(); i++)
	{
		if (fds[i].revents & POLLIN)
		{
			fds[i].revents = 0;
			return readClientMessage(fds[i].fd);
		}
	}
	return NULL;
}
