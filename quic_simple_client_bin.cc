#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"

#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/base/privacy_mode.h"
#include "net/cert/cert_verifier.h"
#include "net/cert/multi_log_ct_verifier.h"
#include "net/http/http_request_info.h"
#include "net/http/transport_security_state.h"
#include "net/log/net_log.h"
#include "net/quic/crypto/proof_verifier_chromium.h"
#include "net/quic/quic_protocol.h"
#include "net/quic/quic_server_id.h"
#include "net/quic/quic_utils.h"
#include "net/spdy/spdy_header_block.h"
#include "net/spdy/spdy_http_utils.h"
#include "net/tools/quic/synchronous_host_resolver.h"
#include "url/gurl.h"

#include "net/tools/quic/d0020e_libquic/quic_simple_client.h"

using base::StringPiece;
using net::CertVerifier;
using net::CTVerifier;
using net::MultiLogCTVerifier;
using net::ProofVerifierChromium;
using net::TransportSecurityState;
using std::cout;
using std::cerr;
using std::map;
using std::string;
using std::vector;
using std::endl;

// The IP or hostname the quic client will connect to.
string FLAGS_host = "localhost";
// The port to connect to.
int32 FLAGS_port = 6121;

string dataToSend = "";
// A semicolon separated list of key:value pairs to add to request headers.
string FLAGS_headers = "";
// Set to true for a quieter output experience.
bool FLAGS_quiet = false;
// QUIC version to speak, e.g. 21. If not set, then all available versions are
// offered in the handshake.
int32 FLAGS_quic_version = -1;
// If true, a version mismatch in the handshake is not considered a failure.
// Useful for probing a server to determine if it speaks any version of QUIC.
bool FLAGS_version_mismatch_ok = false;
// If true, an HTTP response code of 3xx is considered to be a successful
// response, otherwise a failure.
bool FLAGS_redirect_is_success = true;
// Initial MTU of the connection.
int32 FLAGS_initial_mtu = 0;

class FakeCertVerifier: public net::CertVerifier {
public:
	int Verify(net::X509Certificate* cert, const std::string& hostname,
			const std::string& ocsp_response, int flags, net::CRLSet* crl_set,
			net::CertVerifyResult* verify_result,
			const net::CompletionCallback& callback,
			scoped_ptr<net::CertVerifier::Request>* out_req,
			const net::BoundNetLog& net_log) override {
		return net::OK;
	}

	// Returns true if this CertVerifier supports stapled OCSP responses.
	bool SupportsOCSPStapling() override {
		return false;
	}
};

int main(int argc, char *argv[]) {

	base::CommandLine::Init(argc, argv);

	logging::LoggingSettings settings;
	settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
	CHECK(logging::InitLogging(settings));

	base::AtExitManager exit_manager;
	base::MessageLoopForIO message_loop;

	// Determine IP address to connect to from supplied hostname.
	net::IPAddressNumber ip_addr;

	string host = FLAGS_host;
	int port = FLAGS_port;

	if (!net::ParseIPLiteralToNumber(host, &ip_addr)) {
		net::AddressList addresses;
		int rv = net::tools::SynchronousHostResolver::Resolve(host, &addresses);
		if (rv != net::OK) {
			LOG(ERROR) << "Unable to resolve '" << host << "' : "
					<< net::ErrorToShortString(rv);
			return 1;
		}
		ip_addr = addresses[0].address();
	}

	string host_port = net::IPAddressToStringWithPort(ip_addr, FLAGS_port);
	cout << "Resolved " << host << " to " << host_port << endl;

	// Build the client, and try to connect.
	net::QuicServerId server_id(net::IPAddressToString(ip_addr), FLAGS_port,
			net::PRIVACY_MODE_DISABLED);

	net::QuicVersionVector versions = net::QuicSupportedVersions();
	if (FLAGS_quic_version != -1) {
		versions.clear();
		versions.push_back(static_cast<net::QuicVersion>(FLAGS_quic_version));
	}

	// For secure QUIC we need to verify the cert chain.
	scoped_ptr < CertVerifier > cert_verifier(CertVerifier::CreateDefault());
	cert_verifier.reset(new FakeCertVerifier());

	scoped_ptr < TransportSecurityState
			> transport_security_state(new TransportSecurityState);
	scoped_ptr < CTVerifier > ct_verifier(new MultiLogCTVerifier());

	ProofVerifierChromium* proof_verifier = new ProofVerifierChromium(
			cert_verifier.get(), nullptr, transport_security_state.get(),
			ct_verifier.get());



	/**
	 * Starts intresting stuff down here
	 */

	cout << "QuicLib Starts ... !" << endl;

	net::tools::QuicSimpleClient client(net::IPEndPoint(ip_addr, port),
			server_id, versions, proof_verifier);

	client.set_initial_max_packet_length(
			FLAGS_initial_mtu != 0 ?
					FLAGS_initial_mtu : net::kDefaultMaxPacketSize);

	client.Initialize();

	client.set_store_response(true);

	client.Connect();
	cout << "Connected to " << host_port << endl;

	dataToSend = "Hej Servern From Client.";

	client.SendData(dataToSend);
	client.SendData("123");
	client.SendData("456");
	client.SendData("789");
	client.WaitForData();

	cout << endl;
	cout << "Sent from client:" << endl;
	cout << "body: " << dataToSend << endl;
	cout << endl;
	cout << "Response from server:" << endl;
	cout << "body: " << client.latest_response_body() << endl;

	return 0;
}
