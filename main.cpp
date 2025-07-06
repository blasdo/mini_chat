#include "connectionSocket.hpp"
#include <asm-generic/errno.h>
#include <sstream>
#include <sys/poll.h>
# include <iostream>
#include <vector>
#include <cstdlib>
# include <unistd.h>

int main()
{
	int					pollr;
	ConnectionSocket sock(2425);
	NetMessage		*msg;


	while (1)
	{
		sock.WaitNewConnection();
		msg = sock.getNextMessage();
		if (msg)
		{
			std::stringstream ss;
			ss << msg->message << std::endl;
			std::cout << ss.str();
			sock.sendBroadcastMessage(ss.str());
			msg->response("Received!\n");
			delete msg;
		}
		sock.garbageCollector();
	}
}
