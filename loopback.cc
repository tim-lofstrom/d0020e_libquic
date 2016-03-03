#include "loopback.h"
#include <string>

using std::cout;
using std::endl;
using base::StringPiece;

namespace net{
namespace tools{

LoopBack::LoopBack(){
	tv.tv_sec = 2;
	tv.tv_usec = 100000;
	FD_ZERO(&readfds);

	socket_fd = socket(AF_INET,SOCK_STREAM,0); //Creates socket
	if(socket_fd == -1) cout << "socket error" << endl;

	cout << "Socket created" << endl;

	struct sockaddr_in servaddr_in;
	servaddr_in.sin_family = AF_INET;
	servaddr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr_in.sin_port = htons(9999);

	int status_bind = bind(socket_fd, (struct sockaddr *)&servaddr_in, (int)sizeof(servaddr_in) );
	if(status_bind == -1) cout << "bind error" << endl;

	cout << "Socket bind" << endl;

	int status_listen = listen(socket_fd, 0);
	if (status_listen == -1)  cout << "listen error" << endl;

	cout << "Listening" << endl;

	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	socket_fd_read_write = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);
	if ( socket_fd_read_write == -1)
	{
		cout << "listen error" << endl ;
	}
	else
	{
		cout << "Connection accepted. Using new socket : "  <<  socket_fd_read_write << endl;
		FD_SET(socket_fd_read_write, &readfds);
	}

	cout << "Constructor done" << endl;
};

LoopBack::~LoopBack(){
	close(socket_fd);
};

void LoopBack::WriteToIPERF(string data_){
	cout << "WriteToIPERF: "<< data_ << endl;
	send(socket_fd_read_write, data_.c_str(), data_.length(), 0);
};

StringPiece LoopBack::ReadFromIPERF(){
	//select(socket_fd_read_write+1, &readfds, NULL, NULL, &tv);
	//if(FD_ISSET(socket_fd_read_write, &readfds)){
		ssize_t bytes_received = recv(socket_fd_read_write, receive_buffer, 1000, 0);
		cout << "Received: "  << bytes_received << endl;
		cout << "Received: "  << receive_buffer << endl;
		StringPiece recvData = StringPiece(receive_buffer);
		//Send data with QUIC to other end
		//FD_CLR(socket_fd_read_write, &readfds);
		return recvData;
	//}
};

} //namespace tools
} //namespace net
