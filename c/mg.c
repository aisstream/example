#include <mongoose.h>
#include <string.h>
#include <signal.h>

static bool interrupted = false;
static const char* msg = "{ \"APIKey\": \"<YOUR API KEY>\", \"BoundingBoxes\": [[[-11.0, 178.0], [30.0, 74.0]]]}";
static const char* url = "wss://stream.aisstream.io/v0/stream";

static void callback(struct mg_connection* c, int ev, void* ev_data, void* fn_data)
{
    if (ev == MG_EV_OPEN)
    {
        c->is_hexdumping = 1;
    }
    else if (ev == MG_EV_CONNECT)
    {
        struct mg_tls_opts opts;
        memset(&opts, 0, sizeof(struct mg_tls_opts));
        mg_tls_init(c, &opts);
    }
    else if (ev == MG_EV_ERROR)
    {
        MG_ERROR(("%p %s", c->fd, (char*)ev_data));
        interrupted = true;
    }
    else if (ev == MG_EV_WS_OPEN)
    {
        mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // When we get response, print it
        struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
        printf("RX: [%.*s]\n", (int)wm->data.len, wm->data.ptr);
    }
    else if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE)
    {
        interrupted = true;
    }
    else
    {
    }
}

static void sigint_handler(int sig)
{
	interrupted = true;
}

int main(int argc, const char** argv)
{
    signal(SIGINT, sigint_handler);

    struct mg_mgr mgr;        // Event manager
    struct mg_connection* c;  // Client connection
    mg_mgr_init(&mgr);        // Initialise event manager
    mg_log_set(MG_LL_INFO);   // Set log level
    c = mg_ws_connect(&mgr, url, callback, NULL, NULL); // Create client
    while (c && interrupted == false) mg_mgr_poll(&mgr, 100); // Wait for echo
    mg_mgr_free(&mgr);        // Deallocate resources
    return 0;
}
