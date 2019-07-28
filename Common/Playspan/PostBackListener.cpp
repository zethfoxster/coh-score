#include <zmq.h>
#include "../3rdparty/cryptopp/randpool.h"

#include "PostBackListener.h"
#include "JSONParser.h"

#include "components/earray.h"
#include "components/EString.h"
#include "utils/log.h"
#include "utils/timing.h"
#include "endian.h"
#include "MemoryPool.h"

/// Seconds between each relay server advertisement
#define POSTBACK_PING_INTERVAL 1.0f

/// Seconds until the relay server is regarded as dead
#define POSTBACK_RELAY_TIMEOUT 60.0f

/// Number of random bytes to generate for the postback client identity
#define POSTBACK_IDENTITY_BYTES 16

/// How long to wait for messages to be delivered before exiting
#define POSTBACK_ZMQ_LINGER_MS 15000

/// Maximum number of messages to handle per tick
#define POSTBACK_MESSAGES_PER_TICK 10000

#ifdef _FULLDEBUG
	#ifdef _M_AMD64
		#pragma comment(lib,"libzmqd64.lib")
	#else
		#pragma comment(lib,"libzmqd.lib")
	#endif	
#else
	#ifdef _M_AMD64
		#pragma comment(lib,"libzmqr64.lib")
	#else
		#pragma comment(lib,"libzmqr.lib")
	#endif	
#endif

#define MAX_MESSAGE_PARTS 6

#define MSG_ADVERTISE "ADVERTISE"
#define MSG_REGISTERED "REGISTERED"
#define MSG_POSTBACK "POSTBACK"
#define MSG_ACK "ACK"
#define MSG_RECONCILE "RECONCILE"

MP_DEFINE(PostbackAddress);
MP_DEFINE(PostbackMessage);

class MultipartMessage {
public:
	zmq_msg_t chunks[MAX_MESSAGE_PARTS];
	U8 parts;

	MultipartMessage() {
		init();
	}

	~MultipartMessage() {
		close();
	}

	void init() {
		memset(this, 0, sizeof(*this));
	}

	size_t size(unsigned index) {
		if (!devassert(index < parts))
			return NULL;
		return zmq_msg_size(chunks + index);
	}

	void * data(unsigned index) {
		if (!devassert(index < parts))
			return NULL;
		return zmq_msg_data(chunks + index);
	}

	void close() {
		for (unsigned i=0; i<parts; i++) {
			zmq_msg_close(&chunks[i]);
		}
		parts = 0;
	}
};

class Postback {
	/// 0MQ context
	void * mq_context;

	/// 0MQ router connection for message data
	void * mq_client;

	/// 0MQ load-balanced connection for advertisements
	void * mq_dealer;

	/// Random number generator to generate the client identity
	CryptoPP::RandomPool rng;

	/// Client identity
	PostbackAddress client_id;

	/// Last postback identity
	PostbackAddress * last_postback;

	/// Timer for tracking advertisement latencies
	U32 advertisement_timer;

	/// Last advertisement response in @ref timerSecondsSince2000 notation
	U32 last_advertisement_response;

	/// Last advertisement latency in microseconds
	U32 last_advertisement_latency;

	/// Last response in @ref timerSecondsSince2000 notation
	U32 last_received;

	/// Messages received since the last reset
	U32 message_counter;

	// Shallow copy pointers into a configuration structure
	char *mtx_environment;
	char *mtx_secret_key;
	char *playSpan_domain;
	PostBackRelay **postback_relays;
	PostbackCallback postback_callback;

public:
	Postback() : mq_context(NULL), mq_client(NULL), mq_dealer(NULL), last_postback(NULL),
		advertisement_timer(0), last_advertisement_response(0), last_advertisement_latency(0),
		last_received(0), message_counter(0) {}

	void init(int ioThreadCount, char *mtxEnvironment, char *mtxSecretKey, char *playSpanDomain, PostBackRelay **relays, PostbackCallback callback) {
		assert(!mq_context);
		mq_context = zmq_init(ioThreadCount);

		// Seed RNG with CPU ticks
		U64 seed;
		GET_CPU_TICKS_64(seed);
		rng.Put((byte*)&seed, sizeof(seed));

		// ARM NOTE:  I'm commenting this out due to refactoring this code into a library.
		//   Do we need to replace this with something else?
		// Seed RNG with local config file
		//rng.Put((byte*)&g_accountServerState, sizeof(g_accountServerState));

		MP_CREATE(PostbackAddress, 4);
		MP_CREATE(PostbackMessage, 65536);

		advertisement_timer = timerAlloc();

		mtx_environment = mtxEnvironment;
		mtx_secret_key = mtxSecretKey;
		playSpan_domain = playSpanDomain;
		postback_relays = relays;
		postback_callback = callback;
	}

	void shutdown() {		
		zmq_term(mq_context);
		mq_context = NULL;

		MP_DESTROY(PostbackAddress);
		MP_DESTROY(PostbackMessage);

		timerFree(advertisement_timer);
	}

	void connect() {
		int i;	
		int num_relays = eaSize(&postback_relays);

		assert(!mq_client);
		mq_client = zmq_socket(mq_context, ZMQ_ROUTER);
		assert(mq_client);

		int client_linger = POSTBACK_ZMQ_LINGER_MS;
		if (zmq_setsockopt(mq_client, ZMQ_LINGER, &client_linger, sizeof(client_linger)) != 0)
			FatalErrorf("zmq_setsockopt: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));

		// Generate a new client id
		client_id.size = POSTBACK_IDENTITY_BYTES;
		rng.GenerateBlock(client_id.identity, client_id.size);

		// First byte being zero is reserved by ZeroMQ, hardcode to C for debugging
		client_id.identity[0] = 'C';

		if (zmq_setsockopt(mq_client, ZMQ_IDENTITY, client_id.identity, client_id.size) != 0)
			FatalErrorf("zmq_setsockopt: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));

		assert(!mq_dealer);
		mq_dealer = zmq_socket(mq_context, ZMQ_DEALER);
		assert(mq_dealer);

		int dealer_linger = 0;
		if (zmq_setsockopt(mq_dealer, ZMQ_LINGER, &dealer_linger, sizeof(dealer_linger)) != 0)
			FatalErrorf("zmq_setsockopt: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));

		for (i=0; i<num_relays; i++) {
			char connect_str[256];
			PostBackRelay * relay = postback_relays[i];

			snprintf(connect_str, sizeof(connect_str), "tcp://%s:%d", relay->ip, relay->port);
			if (zmq_connect(mq_client, connect_str) != 0)
				FatalErrorf("Could not connect client to '%s': %d %s\n", connect_str, zmq_errno(), zmq_strerror(zmq_errno()));
			if (zmq_connect(mq_dealer, connect_str) != 0)
				FatalErrorf("Could not connect dealer to '%s': %d %s\n", connect_str, zmq_errno(), zmq_strerror(zmq_errno()));
		}
	}

	void disconnect() {
		if (zmq_close(mq_dealer) != 0)
			Errorf("zmq_close: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
		mq_dealer = NULL;

		if (zmq_close(mq_client) != 0)
			Errorf("zmq_close: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
		mq_client = NULL;
	}

	int send_data(void * s, void * data, size_t size, int flags) {
		zmq_msg_t msg;

		int rc = zmq_msg_init_size(&msg, size);
		if (rc != 0) {
			Errorf("zmq_msg_init_size: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
			return rc;
		}

		memcpy(zmq_msg_data(&msg), data, size);

		rc = zmq_send(s, &msg, flags);
		if (rc != 0) {
			Errorf("zmq_send: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
			return rc;
		}

		return rc;
	}

	int send_static_data(void * s, void * data, size_t size, int flags) {
		zmq_msg_t msg;

		int rc = zmq_msg_init_data(&msg, data, size, NULL, NULL);
		if (rc != 0) {
			Errorf("zmq_msg_init_data: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
			return rc;
		}

		rc = zmq_send(s, &msg, flags);
		if (rc != 0) {
			Errorf("zmq_send: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
			return rc;
		}

		return rc;
	}

	bool have_more_messages(void * s) {
		unsigned long events = 0;
		size_t event_bytes = sizeof(events);

		if (zmq_getsockopt(mq_client, ZMQ_EVENTS, &events, &event_bytes) != 0)
			FatalErrorf("zmq_getsockopt: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));

		devassert(event_bytes == sizeof(events));
		return (events & ZMQ_POLLIN) != 0;
	}

	bool have_more_parts(void * s) {
		unsigned long long more = 0;
		size_t more_bytes = sizeof(more);

		if (zmq_getsockopt(mq_client, ZMQ_RCVMORE, &more, &more_bytes) != 0)
			FatalErrorf("zmq_getsockopt: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));

		devassert(more_bytes == sizeof(more));
		return more != 0;
	}

	int recv_multipart(void * s, MultipartMessage * msg, int flags) {
		int rc = -1;
		assert(!msg->parts);

		do {
			if (!devassert(msg->parts < MAX_MESSAGE_PARTS))
				msg->parts = MAX_MESSAGE_PARTS-1;

			zmq_msg_t * m = &msg->chunks[msg->parts];

			rc = zmq_msg_init(m);
			if (rc != 0) {
				Errorf("zmq_msg_init_data: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
				break;
			}

			rc = zmq_recv(s, m, flags);
			if (rc != 0) {
				Errorf("zmq_recv: %d %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
				break;
			}

			msg->parts++;
		} while(have_more_parts(s));

		return rc;
	}

	void send_advertise() {		
		if (send_static_data(mq_dealer, MSG_ADVERTISE, strlen(MSG_ADVERTISE), ZMQ_SNDMORE) != 0) return;
		if (send_static_data(mq_dealer, client_id.identity, client_id.size, ZMQ_SNDMORE) != 0) return;
		if (send_static_data(mq_dealer, mtx_environment, strlen(mtx_environment), 0) != 0) return;

		timerStart(advertisement_timer);
	}

	void handle_registered(MultipartMessage *msg) {
		last_advertisement_latency = timerElapsed(advertisement_timer) * 1000000.0f;
		last_advertisement_response = timerSecondsSince2000();
	}

	void ack(PostbackMessage * message) {
		if (!devassert(message->refCnt > 0))
			return;
		if (!devassert(message->addr))
			return;
		if (!devassert(message->addr->refCnt > 0))
			return;

		if (--message->refCnt != 0)
			return;

		//printf("ack %I64u\n", endianSwapU64(*(U64*)&message->data[0]));

		if (send_static_data(mq_client, message->addr->identity, message->addr->size, ZMQ_SNDMORE) != 0) return;
		if (send_static_data(mq_client, MSG_ACK, strlen(MSG_ACK), ZMQ_SNDMORE) != 0) return;
		if (send_static_data(mq_client, mtx_environment, strlen(mtx_environment), ZMQ_SNDMORE) != 0) return;
		if (send_data(mq_client, message->data, sizeof(message->data), 0) != 0) return;

		if (--message->addr->refCnt == 0) {
			if (message->addr == last_postback)
				last_postback = NULL;

			MP_FREE(PostbackAddress, message->addr);
		}

		MP_FREE(PostbackMessage, message);
	}

	void reconcile(U32 auth_id) {
		char *buf = estrTemp();
		estrPrintf(&buf, "%s.%d", mtx_environment, auth_id);

		if (send_static_data(mq_dealer, MSG_RECONCILE, strlen(MSG_RECONCILE), ZMQ_SNDMORE) != 0) return;
		if (send_static_data(mq_dealer, playSpan_domain, strlen(playSpan_domain), ZMQ_SNDMORE) != 0) return;		
		if (send_static_data(mq_dealer, mtx_secret_key, strlen(mtx_secret_key), ZMQ_SNDMORE) != 0) return;		
		if (send_static_data(mq_dealer, DEFAULT_PLAYSPAN_PARTNER_ID, strlen(DEFAULT_PLAYSPAN_PARTNER_ID), ZMQ_SNDMORE) != 0) return;				
		if (send_data(mq_dealer, buf, estrLength(&buf), 0) != 0) return;

		estrDestroy(&buf);
	}

	void handle_postback(MultipartMessage *msg) {
		// identity
		if (!devassert(msg->size(0) <= SIZEOF2(PostbackAddress,identity)))
			return;

		// Is this postback the same?
		if (!last_postback || last_postback->size != msg->size(0) || memcmp(msg->data(0), last_postback->identity, last_postback->size)!=0) {
			last_postback = MP_ALLOC(PostbackAddress);
			last_postback->refCnt = 0;
			last_postback->size = (U8)(msg->size(0) & 0xFF);
			memcpy(last_postback->identity, msg->data(0), last_postback->size);
		}

		// message id
		if (!devassert(msg->size(2) == SIZEOF2(PostbackMessage,data)))
			return;

		PostbackMessage * message = MP_ALLOC(PostbackMessage);
		memcpy(message->data, msg->data(2), sizeof(message->data));
		message->addr = last_postback;

		//printf("got %I64u\n", endianSwapU64(*(U64*)&message->data[0]));

		// data
		last_postback->refCnt++;
		postback_callback(message, msg->data(3), msg->size(3));

		// delete immediately if the message failed
		if (!message->refCnt) {
			last_postback->refCnt--;
			MP_FREE(PostbackMessage, message);
		}

		message_counter++;
	}

	void recv_messages() {
		static const size_t msg_registered_len = strlen(MSG_REGISTERED);
		static const size_t msg_postback_len = strlen(MSG_POSTBACK);

		int messages_this_tick = 0;
		while (have_more_messages(mq_client)) {
			MultipartMessage msg;

			if (recv_multipart(mq_client, &msg, 0) != 0)
				break;

			if (msg.parts < 2) {
				LOG(LOG_ACCOUNT, LOG_LEVEL_DEBUG, 0, "Invalid message, only has %d parts\n", msg.parts);
				break;
			}

			size_t bytes = msg.size(1);
			void * data = msg.data(1);			

			if (bytes == msg_postback_len && memcmp(data, MSG_POSTBACK, msg_postback_len) == 0)
				handle_postback(&msg);
			else if (bytes == msg_registered_len && memcmp(data, MSG_REGISTERED, msg_registered_len) == 0)
				handle_registered(&msg);
			else
				devassertmsg(0, "No handler for postback message");

			last_received = timerSecondsSince2000();

			if (++messages_this_tick > POSTBACK_MESSAGES_PER_TICK)
				break;
		}
		
	}

	bool tick() {
		// throttle advertisement
		if (timerElapsed(advertisement_timer) > POSTBACK_PING_INTERVAL) {
			// send one advertisement for each server
			for (int i=0; i<eaSize(&postback_relays); i++)
				send_advertise();
		}

		recv_messages();
		
		// Is the thread responding in a timely fashion?
		return (timerSecondsSince2000() - last_advertisement_response) < POSTBACK_RELAY_TIMEOUT;
	}

	void update_stats(unsigned & messages, unsigned & last_ping, unsigned & last_received) {
		messages = message_counter;
		last_ping = last_advertisement_latency;
		last_received = last_advertisement_response;
	}
};

/// Postback worker thread
static Postback *pb = NULL;

C_DECLARATIONS_BEGIN

void postback_start(int ioThreadCount, char *mtxEnvironment, char *mtxSecretKey, char *playSpanDomain, PostBackRelay **relays, PostbackCallback callback) {
	if (!pb)
		pb = new Postback();

	pb->init(ioThreadCount, mtxEnvironment, mtxSecretKey, playSpanDomain, relays, callback);
//	pb->connect();
}

void postback_stop() {
//	pb->disconnect();
	pb->shutdown();
	// delete db;
}

void postback_reconfig() {
//	pb->disconnect();
//	pb->connect();
}

bool postback_tick() {
	return true;
//	return pb->tick();
}

void postback_ack(PostbackMessage * message) {
//	pb->ack(message);
}

void postback_reconcile(U32 auth_id) {
//	pb->reconcile(auth_id);
}

const char * postback_update_stats() {
	static unsigned last_messages = 0;

	unsigned messages = 0;
	unsigned last_ping = 0;
	unsigned last_received = 0;
//	if (pb)
//		pb->update_stats(messages, last_ping, last_received);

	char timestr[64];
	timerMakeOffsetStringFromSeconds(timestr, last_received % 3600);

	static char buf[256];
	snprintf(buf, sizeof(buf), "MTX(T:%d D:%d P:%dus L:%s)", messages, messages-last_messages, last_ping / 1000, timestr);

	last_messages = messages;
	return buf;
}

C_DECLARATIONS_END
