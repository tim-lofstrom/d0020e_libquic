/*
 * loopback.h
 *
 *  Created on: Feb 25, 2016
 *      Author: group7
 */

#ifndef NET_TOOLS_QUIC_D0020E_LIBQUIC_LOOPBACK_H_
#define NET_TOOLS_QUIC_D0020E_LIBQUIC_LOOPBACK_H_

#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "base/logging.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"

using std::string;

namespace net{
namespace tools{

class LoopBack{
public:
	LoopBack();

	~LoopBack();

	void WriteToIPERF(string data);

	base::StringPiece ReadFromIPERF();

private:
	int socket_fd;
	int socket_fd_read_write;
    //char sendline[100];
    char receive_buffer[100];

    struct timeval tv;
    fd_set readfds;
};

} //namespace tools
} //namespace net

#endif /* NET_TOOLS_QUIC_D0020E_LIBQUIC_LOOPBACK_H_ */
