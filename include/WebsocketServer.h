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
	static void setMessageCallback( MessageCallback message_callback );

private:
	static bool getWebsocket(const std::string &id, websocketpp::connection_hdl &hdl);
	static bool getId(const websocketpp::connection_hdl& hdl, std::string &id);

	static websocketpp::server<websocketpp::config::asio> server;
	static std::mutex websocketsLock;
	static std::map<std::string, websocketpp::connection_hdl> g_id_to_socket;
	static std::map<websocketpp::connection_hdl, std::string, std::owner_less<websocketpp::connection_hdl>> g_socket_to_id;

	static MessageCallback g_message_callback;
	//static LogStream ls;
	//static std::ostream os;

	// callbacks
	static bool on_validate(websocketpp::connection_hdl hdl);
	static void on_fail(websocketpp::connection_hdl hdl);
	static void on_close(websocketpp::connection_hdl hdl);
	static void on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
};






#include <thread>
#include <fstream>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <functional>



class WebSocketServerGUI : public QObject
{

	Q_OBJECT
public:
	WebSocketServerGUI():
		QObject(),
		server_thread(0),
		streaming_thread(0)
	{
		start_server();
	}

	~WebSocketServerGUI()
	{
		stop_server();
	}

public slots:
	void start_server(bool state = false)
	{
		if( server_thread )
		{
			std::cout << "WebSocketServerGUI::start_server: server already running\n";std::flush(std::cout);
			return;
		}
		server_thread = new std::thread(WebsocketServer::run);
		//WebsocketServer::run();
		std::cout << "WebSocketServerGUI::start_server: server started\n";std::flush(std::cout);
	}



	void send_data(bool state = false)
	{
		// read file from disk (later we will use data from renderer)
		std::ifstream file( "c:/projects/msc/tests/ws_server/test.jpg", std::ios::binary );

		std::string data((std::istreambuf_iterator<char>(file)),
						 std::istreambuf_iterator<char>());

		std::cout << "size=" << data.size() << std::endl;
		//std::string data = "hello from server!";
		WebsocketServer::sendData( "test", data );

		//std::cout << data << std::endl;
	}

	void start_streaming(bool state = false)
	{
		if(streaming_thread)
		{
			return;
		}

		streaming_thread = new std::thread(streamer);
	}

	void stop_streaming(bool state = false)
	{
		if(!streaming_thread)
		{
			return;
		}
		streaming_thread->join();
		delete streaming_thread;
		streaming_thread = 0;
	}


	void stop_server(bool state = false)
	{
		if( !server_thread )
		{
			std::cout << "WebSocketServerGUI::stop_server: server not running\n";std::flush(std::cout);
			return;
		}
		WebsocketServer::stop();
		server_thread->join();
		delete server_thread;
		server_thread = 0;
		std::cout << "WebSocketServerGUI::stop_server: server stopped\n";std::flush(std::cout);
	}
public:
	std::function<void()> streamer;
private:

	std::thread* server_thread;
	std::thread* streaming_thread;
};
