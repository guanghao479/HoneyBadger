#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

#include "../HBcommon.h"

using namespace std;

int read_out_buffer(struct evbuffer* input, char** precord, uint32_t* precord_len) {
	size_t buffer_len = evbuffer_get_length(input);
	if(buffer_len < sizeof(frame_t)) {
		printd("Warning: now buffer_len=%d, The size field hasn't arrived yet\n", buffer_len);
		return BAD_BUFFER;
	};

	uint32_t record_len; // TODO: endianess?
	evbuffer_copyout(input, &record_len, sizeof(uint32_t));
	if(buffer_len < record_len + 4) {
		printd("Warning: The record hasn't arrived\n");
		return BAD_BUFFER;
	}

	// note: caller responsible for free mem
	char* record = (char*) malloc(record_len);
	if( record == NULL ) return BAD_ADDR;

	evbuffer_drain(input, 4);
	evbuffer_remove(input, record, record_len);

	*precord = record;
	*precord_len = record_len;

	printd("size: %d, content: %s\n", record_len, record);

	return OK;
}

// interpret and process this request in record string
int process_request(char* record, uint32_t* recored_len, string reply_str) {

	reply_str = "REGISTER_OK";

	return OK;
}

/* This callback is invoked when there is data to read on bev. */
	static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
	printd("echo_read_cb():");
	struct evbuffer *input = bufferevent_get_input(bev);
	struct evbuffer *output = bufferevent_get_output(bev);

	// get the buffer size and content out
	char* record;
	uint32_t record_len;
	assert(read_out_buffer(input, &record, &record_len) == OK);

	// process this request
	string reply_str("REGISTER_OK");

	//assert(process_request(record, &record_len, reply_str) == OK);

	//fire off the reply message
	evbuffer_add(output, reply_str.c_str(), reply_str.length());

	free(record);
}

	static void
echo_write_cb(struct bufferevent *bev, void *ctx)
{
	std::cout << "echo_write_cb()" << std::endl;
}
	static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
	printd("echo_event_cb():");
	if (events & BEV_EVENT_ERROR)
		perror("Error from bufferevent");
	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
		bufferevent_free(bev);
		printf("buffer event freed\n");
	}
}

	static void
accept_conn_cb(struct evconnlistener *listener,
		evutil_socket_t fd, struct sockaddr *address, int socklen,
		void *ctx)
{
	cout << "accept_conn_cb(): fd=" << fd <<", "<< "socklen="<<socklen
														 << endl;
	/* We got a new connection! Set up a bufferevent for it. */
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev = bufferevent_socket_new(
			base, fd, BEV_OPT_CLOSE_ON_FREE); // TODO: can't make it thread-safe(thread support not active?)
	assert(bev != NULL);

	bufferevent_setcb(bev, echo_read_cb, echo_write_cb, echo_event_cb, NULL);
	bufferevent_enable(bev, EV_READ|EV_WRITE);
}

	static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	fprintf(stderr, "Got an error %d (%s) on the listener. "
			"Shutting down.\n", err, evutil_socket_error_to_string(err));

	event_base_loopexit(base, NULL);
}

	int
main(int argc, char **argv)
{
	struct event_base *base;
	struct evconnlistener *listener;
	struct sockaddr_in sin;

	int port = kServerPort;

	if (argc > 1) {
		port = atoi(argv[1]);
	}
	if (port<=0 || port>65535) {
		puts("Invalid port");
		return 1;
	}

	base = event_base_new();
	if (!base) {
		puts("Couldn't open event base");
		return 1;
	}

	/* Clear the sockaddr before using it, in case there are extra
	 * platform-specific fields that can mess us up. */
	memset(&sin, 0, sizeof(sin));
	/* This is an INET address */
	sin.sin_family = AF_INET;
	/* Listen on 0.0.0.0 */
	//sin.sin_addr.s_addr = htonl(0);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* Listen on the given port. */
	sin.sin_port = htons(port);

	listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
			LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
			(struct sockaddr*)&sin, sizeof(sin));
	if (!listener) {
		perror("Couldn't create listener");
		return 1;
	}
	evconnlistener_set_error_cb(listener, accept_error_cb);

	event_base_dispatch(base);
	return 0;
}
