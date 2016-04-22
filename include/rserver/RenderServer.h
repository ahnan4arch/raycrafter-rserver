#pragma once


#include <thread>
#include <algorithm>
#include <rserver/WebsocketServer.h>



void write_jpeg_to_memory( int width, int height, const unsigned char* rgb_data, std::string& output );
void write_jpeg_to_file( int width, int height, const unsigned char* rgb_data );


struct ProgressiveRendererInterface
{
	virtual void advance()=0;
	virtual float *getRGBData( int& width, int& height )=0;
	virtual void receiveMessage( const std::string& id, const std::string& message )=0;
};





class RenderServer
{
public:
	RenderServer( ProgressiveRendererInterface* renderer ):
		server_thread(0),
		render_thread(0),
		m_renderer(renderer)
	{
		WebsocketServer::init();
		WebsocketServer::setMessageCallback( std::bind( &RenderServer::on_message, this, std::placeholders::_1, std::placeholders::_2) );
	}

	~RenderServer()
	{
		stop();
	}

	ProgressiveRendererInterface* lock()
	{
		m_renderer_access_lock.lock();
		return m_renderer;
	}
	void unlock()
	{
		m_renderer_access_lock.unlock();
	}


	void start()
	{
		// start server thread ---
		if( server_thread )
		{
			std::cout << "WebSocketServerGUI::start_server: server already running\n";std::flush(std::cout);
			return;
		}
		server_thread = new std::thread(WebsocketServer::run);
		std::cout << "WebSocketServerGUI::start_server: server started\n";std::flush(std::cout);

		// start renderthread ---
		m_render_thread_done = false;
		render_thread = new std::thread(std::bind(&RenderServer::render_loop, this));

	}

	void stop()
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

		// tell the renderthread that we are done...
		m_render_thread_done = true;

		// join render thread...
		render_thread->join();
		delete render_thread;
		render_thread = 0;
		std::cout << "WebSocketServerGUI::stop_server: renderloop stopped\n";std::flush(std::cout);

	}

	void on_message( const std::string& id, const std::string& message )
	{
		lock();
		m_renderer->receiveMessage(id, message);
		unlock();
	}

public:
private:

	void render_loop()
	{
		while(!m_render_thread_done)
		{
			ProgressiveRendererInterface* rinfo = lock();

			// run one iteration ---
			rinfo->advance();

			// get image data (float) and prepare sending to clients ---
			int width,height;
			float* rgb_data = rinfo->getRGBData(width, height);

			// convert from linear to srgb and apply exposure
			std::vector<unsigned char> rgb_data_srgb(width*height*3, 0);
			std::transform( rgb_data, rgb_data+width*height*3, rgb_data_srgb.begin(),
								[](float value_linear)
								{
									float value_srgb = 0;
									if (value_linear <= 0.0031308f)
										value_srgb = 12.92f * value_linear;
									else
										value_srgb = (1.0f + 0.055f)*std::pow(value_linear, 1.0f/2.4f) -  0.055f;

									return (unsigned char)(std::max( 0.0, std::min(255.0, value_srgb*255.0) ));
								});

			// convert to jpeg in memory
			std::string image_data;
			write_jpeg_to_memory( width, height, &rgb_data_srgb[0], image_data );

			unlock();

			// send data ---
			//WebsocketServer::sendData( "test", image_data );
			WebsocketServer::broadcastData( image_data );
		}
		std::cout << "done rendering "  << std::endl;std::flush(std::cout);
	}


	std::thread* server_thread;
	std::thread* render_thread;
	ProgressiveRendererInterface* m_renderer;
	std::mutex m_renderer_access_lock;
	bool m_render_thread_done;
};

/*

#include <thread>
#include <fstream>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <functional>



class RenderServerGUI : public QWidget
{

	Q_OBJECT
public:
	RenderServerGUI( ProgressiveRendererInterface* renderer ):
		QWidget()
	{
		m_rserver = new RenderServer(renderer);
		m_rserver->start();


		QVBoxLayout* vboxlayout = new QVBoxLayout();
		//QPushButton* button_start_server = new QPushButton("start server");
		//QPushButton* button_send_data = new QPushButton("send data");
		QPushButton* button_stop_server = new QPushButton("stop server");
		//vboxlayout->addWidget( button_start_server );
		//vboxlayout->addWidget( button_send_data );
		vboxlayout->addWidget( button_stop_server );

		//WebSocketServerGUI::connect( button_start_server, SIGNAL(clicked(bool)), &gui, SLOT(start_server(bool)) );
		//WebSocketServerGUI::connect( button_send_data, SIGNAL(clicked(bool)), &gui, SLOT(send_data(bool)) );
		RenderServerGUI::connect( button_stop_server, SIGNAL(clicked(bool)), this, SLOT(stop_server(bool)) );


		this->setLayout(vboxlayout);
	}

	~RenderServerGUI()
	{
		delete m_rserver;
	}

	RenderServer* getRServer()
	{
		return m_rserver;
	}

public slots:

	void stop_server(bool state = false)
	{
		m_rserver->stop();
	}
public:
private:
	RenderServer* m_rserver;
};
*/
