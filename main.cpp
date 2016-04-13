#include <iostream>


#include <QApplication>
#include <WebsocketServer.h>

//using namespace std;

#include <libjpeg/jpeglib.h>

// expecting 8bit per channel
void write_jpeg_to_file( int width, int height, const unsigned char* rgb_data )
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	FILE* outfile = fopen("c:/projects/msc/test.jpeg", "wb");

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = width;
	cinfo.image_height     = height;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	// setup ---
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality (&cinfo, 75, true); // quality between 0 and 100

	// start compressing ---
	jpeg_start_compress(&cinfo, true);

	JSAMPROW scanline_pointer;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		scanline_pointer = (JSAMPROW) &rgb_data[cinfo.next_scanline*3*width];
		jpeg_write_scanlines(&cinfo, &scanline_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fclose(outfile);
}

// expecting 8bit per channel
// we write to std::string because this is what websocketpp uses for binary blobs...
void write_jpeg_to_memory( int width, int height, const unsigned char* rgb_data, std::string& output )
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	//FILE* outfile = fopen("c:/projects/msc/test.jpeg", "wb");
	//jpeg_stdio_dest(&cinfo, outfile);

	unsigned char *mem = NULL;
	unsigned long mem_size = 0;
	jpeg_mem_dest(&cinfo, &mem, &mem_size);

	cinfo.image_width      = width;
	cinfo.image_height     = height;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	// setup ---
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality (&cinfo, 75, true); // quality between 0 and 100

	// start compressing ---
	jpeg_start_compress(&cinfo, true);

	JSAMPROW scanline_pointer;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		scanline_pointer = (JSAMPROW) &rgb_data[cinfo.next_scanline*3*width];
		jpeg_write_scanlines(&cinfo, &scanline_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	output = std::string( mem, mem+mem_size );

	free(mem);
}


struct ImageStream
{
	void lock()
	{
		m_access_lock.lock();
	}

	void unlock()
	{
		m_access_lock.unlock();
	}

	int width;
	int height;
	std::vector<unsigned char> rgb_data;

private:
	std::mutex m_access_lock;
};

ImageStream g_image_stream;

double g_t = 0.0;
void render_loop()
{
	bool condition = true;
	while(condition)
	{
		g_image_stream.lock();

		std::fill(g_image_stream.rgb_data.begin(), g_image_stream.rgb_data.end(), 0);

		double cx = 0.5 + 0.3*std::cos(g_t);
		double cy = 0.5 - 0.3*std::sin(g_t);
		g_t += 0.1;

		// do some shit
		for( int j=0;j<g_image_stream.height;++j )
			for( int i=0;i<g_image_stream.width;++i )
			{
				double u = double(i)/double(g_image_stream.width-1);
				double v = double(j)/double(g_image_stream.height-1);

				double x = u-cx;
				double y = v-cy;
				double d = std::sqrt( x*x+y*y );
				if( d < 0.1 )
				{
					g_image_stream.rgb_data[ j*g_image_stream.width*3+i*3 + 0 ] = 255;
					g_image_stream.rgb_data[ j*g_image_stream.width*3+i*3 + 1 ] = 255;
					g_image_stream.rgb_data[ j*g_image_stream.width*3+i*3 + 2 ] = 255;
				}
			}

		g_image_stream.unlock();
	}
}

void streaming_loop()
{
	bool condition = true;
	while( condition )
	{
		g_image_stream.lock();
	/*
		//write_jpeg_to_file( g_image_stream.width, g_image_stream.height, &g_image_stream.rgb_data[0] );
	*/
		std::string image_data;
		write_jpeg_to_memory( g_image_stream.width, g_image_stream.height, &g_image_stream.rgb_data[0], image_data );
		g_image_stream.unlock();
		WebsocketServer::sendData( "test", image_data );

	};
}


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	g_image_stream.width = 260;
	g_image_stream.height = 480;
	g_image_stream.rgb_data = std::vector<unsigned char>( g_image_stream.width*g_image_stream.height*3, 0 );


	WebsocketServer::init();

	WebSocketServerGUI gui;
	gui.streamer = std::bind(streaming_loop);

	QWidget widget;
	QVBoxLayout* vboxlayout = new QVBoxLayout();
	QPushButton* button_start_server = new QPushButton("start server");
	QPushButton* button_send_data = new QPushButton("send data");
	QPushButton* button_start_streaming = new QPushButton("start streaming");
	QPushButton* button_stop_streaming = new QPushButton("stop streaming");
	QPushButton* button_stop_server = new QPushButton("stop server");
	vboxlayout->addWidget( button_start_server );
	vboxlayout->addWidget( button_send_data );
	vboxlayout->addWidget( button_start_streaming );
	vboxlayout->addWidget( button_stop_streaming );
	vboxlayout->addWidget( button_stop_server );

	WebSocketServerGUI::connect( button_start_server, SIGNAL(clicked(bool)), &gui, SLOT(start_server(bool)) );
	WebSocketServerGUI::connect( button_send_data, SIGNAL(clicked(bool)), &gui, SLOT(send_data(bool)) );
	WebSocketServerGUI::connect( button_start_streaming, SIGNAL(clicked(bool)), &gui, SLOT(start_streaming(bool)) );
	WebSocketServerGUI::connect( button_stop_streaming, SIGNAL(clicked(bool)), &gui, SLOT(stop_streaming(bool)) );
	WebSocketServerGUI::connect( button_stop_server, SIGNAL(clicked(bool)), &gui, SLOT(stop_server(bool)) );


	widget.setLayout(vboxlayout);
	widget.show();



	// start rendering ...
	std::thread render_thread( render_loop );









	/*
	//std::cout << "Hello World!" << __cplusplus  << " " << _MSC_VER << std::endl;
	WebsocketServer::init();
	WebsocketServer::run();
	WebsocketServer::stop();
	*/

	int result = app.exec();

	return result;

	//return 0;
}

