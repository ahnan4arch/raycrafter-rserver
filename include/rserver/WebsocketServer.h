#pragma once
#include <iostream>
#include <mutex>
#include <functional>


#define ASIO_STANDALONE
#define _WEBSOCKETPP_NOEXCEPT_TOKEN_
#define _WEBSOCKETPP_CONSTEXPR_TOKEN_
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>





class WebsocketServer
{
public:
	typedef std::function<void (const std::string&, const std::string&)> MessageCallback;
	static bool init();
	static void run();
	static void stop();

	static bool sendClose(std::string id);
	static bool sendText(std::string id, std::string text);
	static bool sendData(std::string id, std::string data);
	static bool broadcastData(std::string data);
	static void setMessageCallback( MessageCallback message_callback );

private:
	static bool getWebsocket(const std::string &id, websocketpp::connection_hdl &hdl);
	static bool getId(const websocketpp::connection_hdl& hdl, std::string &id);

	static websocketpp::server<websocketpp::config::asio> server;
	static std::mutex websocketsLock;
	static std::map<std::string, websocketpp::connection_hdl> g_id_to_socket;
	static std::map<websocketpp::connection_hdl, std::string, std::owner_less<websocketpp::connection_hdl>> g_socket_to_id;

	static MessageCallback g_message_callback;

	// callbacks
	static bool on_validate(websocketpp::connection_hdl hdl);
	static void on_fail(websocketpp::connection_hdl hdl);
	static void on_close(websocketpp::connection_hdl hdl);
	static void on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
};





