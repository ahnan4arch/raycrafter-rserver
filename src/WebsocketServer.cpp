#include <WebsocketServer.h>


#include <thread>




websocketpp::server<websocketpp::config::asio> WebsocketServer::server;
std::mutex WebsocketServer::websocketsLock;
std::map<std::string, websocketpp::connection_hdl> WebsocketServer::g_id_to_socket;
std::map<websocketpp::connection_hdl, std::string, std::owner_less<websocketpp::connection_hdl>> WebsocketServer::g_socket_to_id;
//static LogStream ls;
//std::ostream WebsocketServer::os;

WebsocketServer::MessageCallback WebsocketServer::g_message_callback;

bool WebsocketServer::init()
{
	std::cout << "init\n";std::flush(std::cout);


	// Initialising WebsocketServer.
	server.init_asio();

	// disable logging for frame headers and payload...
	server.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);

	// Set custom logger (ostream-based).
	//server.get_alog().set_ostream(&os);
	//server.get_elog().set_ostream(&os);
	server.get_alog().set_ostream(&std::cout);
	server.get_elog().set_ostream(&std::cerr);

	// Register the message handlers.
	server.set_validate_handler(&WebsocketServer::on_validate);
	server.set_fail_handler(&WebsocketServer::on_fail);
	server.set_close_handler(&WebsocketServer::on_close);
	server.set_message_handler(&WebsocketServer::on_message);

	// Listen on port.
	int port = 8082;
	try
	{
		server.listen(port);
	} catch(websocketpp::exception const &e)
	{
		// Websocket exception on listen. Get char string via e.what().
	}

	// Starting Websocket accept.
	websocketpp::lib::error_code ec;
	server.start_accept(ec);
	if (ec)
	{
		// Can log an error message with the contents of ec.message() here.
		return false;
	}

	return true;
}

void WebsocketServer::setMessageCallback( MessageCallback message_callback )
{
	g_message_callback = message_callback;
}

void WebsocketServer::run()
{
	std::cout << "run\n";std::flush(std::cout);
	try
	{
		server.run();
	} catch(websocketpp::exception const &e)
	{
		// Websocket exception. Get message via e.what().
		std::cout << "exception\n";
	}
}

void WebsocketServer::stop()
{
	std::cout << "stop\n";std::flush(std::cout);
	// Stopping the Websocket listener and closing outstanding connections.
	websocketpp::lib::error_code ec;
	server.stop_listening(ec);
	if (ec)
	{
		// Failed to stop listening. Log reason using ec.message().
		return;
	}

	// Close all existing websocket connections.
	std::string data = "Terminating connection...";
	std::map<std::string, websocketpp::connection_hdl>::iterator it;
	for (it = g_id_to_socket.begin(); it != g_id_to_socket.end(); ++it)
	{
		websocketpp::lib::error_code ec;
		server.close(it->second, websocketpp::close::status::normal, data, ec); // send text message.
		if (ec)
		{ // we got an error
			// Error closing websocket. Log reason using ec.message().
		}
	}

	// Stop the endpoint.
	server.stop();
}


bool WebsocketServer::on_validate(websocketpp::connection_hdl hdl)
{
	std::cout << "on_validate\n";std::flush(std::cout);
	websocketpp::server<websocketpp::config::asio>::connection_ptr con = server.get_con_from_hdl(hdl);
	websocketpp::uri_ptr uri = con->get_uri();
	std::string query = uri->get_query(); // returns empty string if no query string set.
	std::cout << "WebsocketServer::on_validate querystring=" << query << std::endl;
	std::string id = "";
	if (!query.empty())
	{
		// Split the query parameter string here, if desired.
		// We assume we extracted a string called 'id' here.
		id = "test";
		std::cout << "warning: id is hardcoded:" << id << std::endl;
	}
	else
	{
		// Reject if no query parameter provided, for example.
		return false;
	}

	websocketsLock.lock();
	g_id_to_socket.insert(std::pair<std::string, websocketpp::connection_hdl>(id, hdl));
	g_socket_to_id.insert(std::pair<websocketpp::connection_hdl, std::string>(hdl, id));
	websocketsLock.unlock();

	return true;
}

void WebsocketServer::on_fail(websocketpp::connection_hdl hdl)
{
	std::cout << "on_fail\n";std::flush(std::cout);
	websocketpp::server<websocketpp::config::asio>::connection_ptr con = server.get_con_from_hdl(hdl);
	websocketpp::lib::error_code ec = con->get_ec();
	// Websocket connection attempt by client failed. Log reason using ec.message().
}

void WebsocketServer::on_close(websocketpp::connection_hdl hdl)
{
	std::cout << "on_close\n";std::flush(std::cout);
	// Websocket connection closed.
}

void WebsocketServer::on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg)
{
	std::cout << "on_message\n";std::flush(std::cout);
	std::string id;
	if( !getId(hdl, id) )
	{
		std::cout << "WebsocketServer::on_message: unable to retrieve id from socket.\n";std::flush(std::cout);
		return;
	}

	const std::string& payload = msg->get_payload();
	if(g_message_callback)
	{
		g_message_callback(id, payload);
	}
}

bool WebsocketServer::getWebsocket(const std::string &id, websocketpp::connection_hdl &hdl)
{
	bool result = true;
	websocketsLock.lock();
	auto it = g_id_to_socket.find(id);
	if(it!=g_id_to_socket.end())
		hdl = it->second;
	else
		result = false;
	websocketsLock.unlock();
	return result;
}

bool WebsocketServer::getId(const websocketpp::connection_hdl &hdl, std::string &id)
{
	bool result = true;
	websocketsLock.lock();
	auto it = g_socket_to_id.find(hdl);
	if(it!=g_socket_to_id.end())
		id = it->second;
	else
		result = false;
	websocketsLock.unlock();
	return result;
}


bool WebsocketServer::sendText(std::string id, std::string text)
{
	websocketpp::connection_hdl hdl;
	if (!getWebsocket(id, hdl))
	{
		// Sending to non-existing websocket failed.
		std::cout << "WebsocketServer::sendText: Sending to non-existing websocket failed.\n";std::flush(std::cout);
		return false;
	}

	websocketpp::lib::error_code ec;
	server.send(hdl, text, websocketpp::frame::opcode::text, ec); // send text message.
	if (ec)
	{ // we got an error
		// Error sending on websocket. Log reason using ec.message().
		std::cout << "WebsocketServer::sendText: Error sending on websocket: " + ec.message();std::flush(std::cout);
		return false;
	}

	return true;
}

bool WebsocketServer::sendData(std::string id, std::string data)
{
	websocketpp::connection_hdl hdl;
	if (!getWebsocket(id, hdl))
	{
		// Sending to non-existing websocket failed.
		std::cout << "WebsocketServer::sendData: Sending to non-existing websocket failed.\n";std::flush(std::cout);
		return false;
	}

	websocketpp::lib::error_code ec;
	server.send(hdl, data, websocketpp::frame::opcode::BINARY, ec); // send text message.
	if (ec)
	{ // we got an error
		// Error sending on websocket. Log reason using ec.message().
		std::cout << "WebsocketServer::sendData: Error sending on websocket: " + ec.message();std::flush(std::cout);
		return false;
	}

	return true;
}

bool WebsocketServer::sendClose(std::string id)
{
	websocketpp::connection_hdl hdl;
	if (!getWebsocket(id, hdl))
	{
		// Closing non-existing websocket failed.
		return false;
	}

	std::string data = "Terminating connection...";
	websocketpp::lib::error_code ec;
	server.close(hdl, websocketpp::close::status::normal, data, ec); // send close message.
	if (ec)
	{ // we got an error
		// Error closing websocket. Log reason using ec.message().
		return false;
	}

	// Remove websocket from the map.
	websocketsLock.lock();
	//pthread_rwlock_rdlock(&websocketsLock);
	g_id_to_socket.erase(id);
	g_socket_to_id.erase(hdl);
	//pthread_rwlock_unlock(&websocketsLock);
	websocketsLock.unlock();

	return true;
}
