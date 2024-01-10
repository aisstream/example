#include <libwebsockets.h>
#include <string.h>
#include <signal.h>

static struct lws_context* context;
static int interrupted = 0;
static const char* msg = "{ \"APIKey\": \"<YOUR API KEY>\", \"BoundingBoxes\": [[[-11.0, 178.0], [30.0, 74.0]]]}";

static int connect_client()
{
	struct lws_client_connect_info i;

	memset(&i, 0, sizeof(i));

	i.context = context;
	i.port = 443;
	i.address = "stream.aisstream.io";
	i.path = "/v0/stream";
	i.host = i.address;
	i.origin = i.address;
	i.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
	i.protocol = "lws-minimal";
	i.local_protocol_name = i.protocol;

	lwsl_notice("%s: connection %s:%d\n", __func__, i.address, i.port);
	if (!lws_client_connect_via_info(&i)) return 1;

	return 0;
}

static int callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
	int m = 0, n = 0;
	short r;

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
		connect_client();
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char*)in : "(null)");
		interrupted = 1;
		break;

		/* --- client callbacks --- */

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lws_callback_on_writable(wsi);
		lwsl_user("%s: established connection, wsi = %p\n", __func__, wsi);
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		lwsl_user("%s: CLOSED\n", __func__);
		interrupted = 1;
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		n = (int)strlen(msg);
		m = lws_write(wsi, (unsigned char*)msg, n, LWS_WRITE_TEXT);
		if (m < n)
		{
			lwsl_err("sending message failed: %d < %d\n", m, n);
			return -1;
		}
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		lwsl_user("RX: %s\n", (const char*)in);
		break;

	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		lwsl_notice("server initiated connection close: len = %lu, in = %s\n", (unsigned long)len, (char*)in);
		return 0;

	default:
		break;
	}

	return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] = {
	{ "rx-tx", callback, 4096, 4096, 0, NULL, 0 },
	LWS_PROTOCOL_LIST_TERM
};

static void sigint_handler(int sig)
{
	interrupted = 1;
}

int main(int argc, const char** argv)
{
	struct lws_context_creation_info info;
	int n = 0;

	signal(SIGINT, sigint_handler);

	lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE, NULL);

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
	info.protocols = protocols;
#if defined(LWS_WITH_MBEDTLS) || defined(USE_WOLFSSL)
	/*
	 * OpenSSL uses the system trust store.  mbedTLS has to be told which
	 * CA to trust explicitly.
	 */
	info.client_ssl_ca_filepath = "./stream.aisstream.io.cer";
#endif
	info.fd_limit_per_thread = (unsigned int)3;

	context = lws_create_context(&info);
	if (!context)
	{
		lwsl_err("lws init failed\n");
		return 1;
	}

	while (n >= 0 && !interrupted)
		n = lws_service(context, 0);

	lwsl_notice("%s: exiting service loop. n = %d, interrupted = %d\n", __func__, n, interrupted);

	lws_context_destroy(context);

	return 0;
}
