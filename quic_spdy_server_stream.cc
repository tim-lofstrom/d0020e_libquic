// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "net/tools/quic/d0020e_libquic/quic_spdy_server_stream.h"

#include "base/logging.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "net/quic/quic_flags.h"
#include "net/tools/quic/d0020e_libquic/quic_spdy_session.h"
#include "net/tools/quic/d0020e_libquic/quic_spdy_stream.h"
#include "net/quic/spdy_utils.h"
#include "net/spdy/spdy_protocol.h"
#include "net/tools/quic/quic_in_memory_cache.h"

using base::StringPiece;
using base::StringToInt;
using std::string;
using std::endl;
using std::cout;

namespace net {
namespace tools {

QuicSpdyServerStream::QuicSpdyServerStream(QuicStreamId id,
                                           QuicSpdySession* session)
    : QuicSpdyStream(id, session), content_length_(-1) {
//	this->lb = lb;
}

QuicSpdyServerStream::~QuicSpdyServerStream() {
}


void QuicSpdyServerStream::OnDataAvailable() {

  while (HasBytesToRead()) {
    struct iovec iov;
    if (GetReadableRegions(&iov, 1) == 0) {
      // No more data to read.
      break;
    }
    body_.append(static_cast<char*>(iov.iov_base), iov.iov_len);

//    if (content_length_ >= 0 &&
//        static_cast<int>(body_.size()) > content_length_) {
//      DVLOG(1) << "Body size (" << body_.size() << ") > content length ("
//               << content_length_ << ").";
//      cout << "fail server " << endl;
//      return;
//    }
    MarkConsumed(iov.iov_len);
  }
  if (!sequencer()->IsClosed()) {
    sequencer()->SetUnblocked();
    return;
  }

  // If the sequencer is closed, then all the body, including the fin, has been
  // consumed.





  cout << "Server Received ";
  printf("%.*s", 4, body_.c_str());

  cout << " with size " << body_.length() << endl;

  OnFinRead();

  if (write_side_closed() || fin_buffered()) {
    return;
  }

  char test[4];
  std::string str = body_.c_str();
  str.copy(test, 4);


  SendEcho(test);
  return;
}

void QuicSpdyServerStream::SendEcho(string data){

	WriteOrBufferData("ACK: ", false, nullptr);
	WriteOrBufferData(data, true, nullptr);
	return;

//	lb->WriteToIPERF(data);
//	WriteOrBufferData(lb->ReadFromIPERF(), true, nullptr);
//
//	return;

	 if (!reading_stopped()) {
	    StopReading();
	  }

//	  fake_WriteHeaders(false);

	  if (!data.empty()) {
		  WriteOrBufferData("ECHO: ", false, nullptr);
		  WriteOrBufferData(data, true, nullptr);
	  }
}

}  // namespace tools
}  // namespace net
