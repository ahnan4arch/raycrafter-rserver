#pragma once


#include <thread>
#include <algorithm>
#include <functional>
#include <rserver/WebsocketServer.h>


void write_jpeg_to_memory( int width, int height, const unsigned char* rgb_data, std::string& output );
void write_jpeg_to_file( const char* filename, int width, int height, const unsigned char* rgb_data );


struct ProgressiveRendererInterface
{
	typedef std::function<void(void)> ProgressCallback;
	//typedef std::function<void(int, int, unsigned char*)> SendImageCallback;

	virtual void setProgressCallback(ProgressCallback callback)=0;
	virtual void getImageResolution(int& width, int& height)=0;
	virtual void copyRGBData( unsigned char* rgb_data, int& width, int& height )=0;
	//virtual void copyRGBData( float *rgb_data )=0;
	virtual void receiveMessage( const std::string& id, const std::string& message )=0;
};


class RenderServer
{
public:
	RenderServer( ProgressiveRendererInterface* renderer ):
		server_thread(0),
		m_renderer(renderer)
	{
		WebsocketServer::init();
		WebsocketServer::setMessageCallback( std::bind( &RenderServer::on_message, this, std::placeholders::_1, std::placeholders::_2) );
		renderer->setProgressCallback( std::bind( &RenderServer::on_progress, this) );
	}

	~RenderServer()
	{
		stop();
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
		std::cout << "RenderServer::start: server running...\n";std::flush(std::cout);
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
	}

	void on_message( const std::string& id, const std::string& message )
	{
		m_renderer->receiveMessage(id, message);
	}

	void on_progress()
	{
		// get image data (float) and prepare sending to clients ---
		//int width,height;
		//m_renderer->getImageResolution( width, height );



		int max_width = 512;
		int max_height = 512;
		std::vector<unsigned char> rgb_data(max_width*max_height*3);
		int width, height;
		m_renderer->copyRGBData(&rgb_data[0], width, height);

		// convert to jpeg in memory
		std::string image_data;
		if( (width!=0) && (height!=0))
			write_jpeg_to_memory( width, height, &rgb_data[0], image_data );

		/*
		std::vector<float> rgb_data_linear(width*height*3);
		m_renderer->copyRGBData(&rgb_data_linear[0]);

		std::string image_data;
		{
			// convert from linear to srgb and apply exposure
			std::vector<unsigned char> rgb_data_srgb(width*height*3, 0);
			std::transform( rgb_data_linear.begin(), rgb_data_linear.end(), rgb_data_srgb.begin(),
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
			write_jpeg_to_memory( width, height, &rgb_data_srgb[0], image_data );
		}
		*/

		// send data ---
		if(!image_data.empty())
			WebsocketServer::broadcastData( image_data );
	}

public:
private:
	std::thread* server_thread;
	ProgressiveRendererInterface* m_renderer;
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
