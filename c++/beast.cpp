#ifdef _WIN32
#include <sdkddkver.h>
#endif
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{
private:
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> m_ws;
    boost::beast::flat_buffer m_buffer;
    std::string m_host;
    std::string m_text;

private:
    // Report a failure
    void fail(boost::beast::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

public:
    // Resolver and socket require an io_context
    explicit session(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx)
        : m_resolver(boost::asio::make_strand(ioc))
        , m_ws(boost::asio::make_strand(ioc), ctx)
        , m_host("stream.aisstream.io")
        , m_text("{ \"APIKey\": \"<YOUR API KEY>\", \"BoundingBoxes\": [[[-11.0, 178.0], [30.0, 74.0]]]}")
    {
    }

    // Start the asynchronous operation
    void run()
    {
        // Look up the domain name
        m_resolver.async_resolve(m_host, "443", boost::beast::bind_front_handler(&session::on_resolve, shared_from_this()));
    }

    void on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
    {
        if (ec)
            return fail(ec, "resolve");

        // Set a timeout on the operation
        boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        boost::beast::get_lowest_layer(m_ws).async_connect(results, boost::beast::bind_front_handler(&session::on_connect, shared_from_this()));
    }

    void on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep)
    {
        if (ec)
            return fail(ec, "connect");

        // Set a timeout on the operation
        boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), m_host.c_str()))
        {
            ec = boost::beast::error_code(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
            return fail(ec, "connect");
        }

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        m_host += ':' + std::to_string(ep.port());

        // Perform the SSL handshake
        m_ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client, boost::beast::bind_front_handler(&session::on_ssl_handshake, shared_from_this()));
    }

    void on_ssl_handshake(boost::beast::error_code ec)
    {
        if (ec)
            return fail(ec, "ssl_handshake");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        boost::beast::get_lowest_layer(m_ws).expires_never();

        // Set suggested timeout settings for the websocket
        m_ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        m_ws.set_option(boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req)
        {
            req.set(boost::beast::http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async-ssl");
        }));

        // Perform the websocket handshake
        m_ws.async_handshake(m_host, "/v0/stream", boost::beast::bind_front_handler(&session::on_handshake,shared_from_this()));
    }

    void on_handshake(boost::beast::error_code ec)
    {
        if (ec)
            return fail(ec, "handshake");

        // Send the message
        m_ws.async_write(boost::asio::buffer(m_text), boost::beast::bind_front_handler(&session::on_write, shared_from_this()));
    }

    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        // Read a message into our buffer
        m_ws.async_read(m_buffer, boost::beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
        {
            fail(ec, "read");

            // Close the WebSocket connection
            m_ws.async_close(boost::beast::websocket::close_code::normal, boost::beast::bind_front_handler(&session::on_close, shared_from_this()));
        }
        else
        {
            // The make_printable() function helps print a ConstBufferSequence
            std::cout << boost::beast::make_printable(m_buffer.data()) << std::endl;

            m_buffer.clear();
            m_ws.async_read(m_buffer, boost::beast::bind_front_handler(&session::on_read, shared_from_this()));
        }
    }

    void on_close(boost::beast::error_code ec)
    {
        if (ec)
            return fail(ec, "close");

        // If we get here then the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // The SSL context is required, and holds certificates
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };

    // Launch the asynchronous operation
    std::make_shared<session>(ioc, ctx)->run();

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}
