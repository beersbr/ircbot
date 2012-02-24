#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include <boost/regex.hpp>
#include "colors.h"
using std::string;

void error(string msg){
	string red = Colors::Red;
	string reset = Colors::reset;
	msg = red+msg+reset;

    perror(msg.c_str());

    exit(0);
}

class irc_bot{
public:
	std::string _server;
	std::string _nick;
	std::string _channel;

	int _port;
    int sockfd, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    /*
    *
    *	The default constructor: used for just a clean object. does nothing by itself. You will have to set everything
    *   up by hand if this one is used.
    *
    */
	irc_bot(){
		this->_server = "";
		this->_port = 0;
		this->_nick = "";
		this->_channel = "";

		this->n = 0;
		this->sockfd = 0;
		this->server = NULL;
	};

	irc_bot(std::string server, int port, std::string nick, std::string channel){
		// setup the regular expressions.
		reg_PONG = "";

		this->_server = server;
		this->_port = port;
		this->_nick = nick;
		this->_channel = channel;
		this->n = 0;

		this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(this->sockfd < 0) error("Error opening the socket");

		this->server = gethostbyname(this->_server.c_str());
		if(this->server == NULL) {
        	std::cerr << Colors::Red << "No such host: " << Colors::reset << this->_server << std::endl;
        	exit(0);
        }

        bzero((char *)&this->serv_addr, sizeof(this->serv_addr));
        this->serv_addr.sin_family = AF_INET;

    	bcopy((char *)this->server->h_addr, (char *)&this->serv_addr.sin_addr.s_addr, this->server->h_length);
    	this->serv_addr.sin_port = htons(this->_port);

    	if(connect(this->sockfd, (struct sockaddr *)&this->serv_addr, sizeof(this->serv_addr)) < 0)
        	error("ERROR connecting");

        // set the socket to non blocking
        fcntl(this->sockfd, F_SETFL, O_NONBLOCK);

		start();
    }

	~irc_bot(){
		if(this->sockfd) close(this->sockfd);
	};

	int send(string s){
		std::cout << Colors::Green << "Sending: " << Colors::reset << s;
		int n = write(this->sockfd, (char*)s.c_str(), strlen(s.c_str()));
		// std::cout << "written: " << n << std::endl;
		return n;
	}

	int start(){
		int running = true;
		int fd = 0;

		fd_set fds;
		fd_set list;
		FD_ZERO(&fds);
		FD_SET(this->sockfd, &fds);
		FD_SET(fileno(stdin), &fds);

		/* Now we have to write stuff to the server */
        std::cout << Colors::Green << "Connecting to: " << Colors::reset << this->_server << ":" << this->_port << std::endl;
        send("USER blah blah blah :blah blah\n");

        std::string nick_string = (std::string)"NICK " + this->_nick + (std::string)"\n";
        send(nick_string);

        std::string channel_string = (std::string)"JOIN "+this->_channel+(std::string)"\n";
        send(channel_string);

        std::string buffer = "";
		while(running){
			list = fds;
			fd = select(this->sockfd+1, &list, NULL, NULL, NULL);

			//check each descriptor for activity
			if(FD_ISSET(this->sockfd, &list)){
				int n = 0;
				buffer = "";

				do{
					char token[2048];
					n = read(this->sockfd, (char*)token, 2048);
					if(n < 2048) token[n] = '\0';
					buffer += token;
					bzero((char*)token, 2048);
				}while(n > 0);

				std::cout << Colors::On_Blue << buffer << Colors::reset << std::endl;
				handle_input(buffer);
				buffer.erase(0, buffer.length());
			}
			if(FD_ISSET(fileno(stdin), &list)){
				char buf[2048];
				fgets((char*)buf, 2048, stdin);
				send(buf);
				bzero((char*)buf, 2048);

			}
		}
	};

private:
	boost::regex reg_PONG;
	boost::regex reg_PING;

	int handle_input(std::string input){
		if(input.empty()) return 0;


	}

};

/**************************************************************************
* Main: Where the bot should be called. I know you are sad that
* I put everything in a single file. Whoops.
**************************************************************************/

int main(int argc, char **argv, char **arge){

	irc_bot bot("chat.freenode.net", 6667, "scremmy", "#scrambles");

	return 0;
}

