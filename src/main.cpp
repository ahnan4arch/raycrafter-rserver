#include <iostream>


#include <QApplication>
#include <WebsocketServer.h>

//using namespace std;

#include <libjpeg/jpeglib.h>

#include <jsmn/jsmn.h>


std::string jsmn_typename( jsmntype_t type )
{
	switch(type)
	{
	case JSMN_UNDEFINED:return "JSMN_UNDEFINED";break;
	case JSMN_OBJECT:return "JSMN_OBJECT";break;
	case JSMN_ARRAY:return "JSMN_ARRAY";break;
	case JSMN_STRING:return "JSMN_STRING";break;
	case JSMN_PRIMITIVE:return "JSMN_PRIMITIVE";break;
	};
	return "";
}

std::string jsmn_to_string( const std::string& json, jsmntok_t* t )
{
	return json.substr(t->start, t->end-t->start);
}

// skips jumps to the next key token with an object
int jsmn_next_key( jsmntok_t* key_token )
{
	jsmntok_t* value_token = key_token+1;

	if( value_token->type == JSMN_PRIMITIVE )
		return 2;
	else
	if( value_token->type == JSMN_STRING )
		return 2;
	else
	if( value_token->type == JSMN_OBJECT )
	{
		int next_key = 1;
		for( int i=0;i<value_token->size;++i )
			next_key = jsmn_next_key( value_token+next_key );
		return next_key+1;
	}

	return 0;
}

void jsmn_to_map( const std::string& json, jsmntok_t* t, std::map<std::string, jsmntok_t*>& map )
{
	int next_key = 1;
	for( int i=0;i<t->size;++i )
	{
		std::string key = jsmn_to_string(json, t+next_key);
		map[key] = t+next_key+1;
		next_key += jsmn_next_key(t+next_key);
	}
}


template<class T>
T jsmn_to_number( const std::string& json, jsmntok_t* t)
{
	std::istringstream stream (json.substr(t->start, t->end-t->start));
	T value;
	stream >> value;
	return value;
}

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


struct RenderInfo
{
	RenderInfo()
	{
		width = 512;
		height = 512;
		rgb_data = std::vector<unsigned char>( width*height*3, 0 );

		t = 0.0;
		cx = 0.5 + 0.3*std::cos(t);
		cy = 0.5 - 0.3*std::sin(t);
	}

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


	double t;
	double cx, cy;
private:
	std::mutex m_access_lock;
};

RenderInfo g_render_info;

void render_loop()
{
	bool condition = true;
	while(condition)
	{
		g_render_info.lock();

		std::fill(g_render_info.rgb_data.begin(), g_render_info.rgb_data.end(), 0);

		g_render_info.t += 0.1;

		// do some shit
		for( int y=0;y<g_render_info.height;++y )
			for( int x=0;x<g_render_info.width;++x )
			{
				//double u = double(i)/double(g_render_info.width-1);
				//double v = double(j)/double(g_render_info.height-1);

				//double x = u-g_render_info.cx;
				//double y = v-g_render_info.cy;
				double x2 = x-g_render_info.cx;
				double y2 = y-g_render_info.cy;
				double d = std::sqrt( x2*x2+y2*y2 );
				if( d < 20.0 )
				{
					g_render_info.rgb_data[ y*g_render_info.width*3+x*3 + 0 ] = 255;
					g_render_info.rgb_data[ y*g_render_info.width*3+x*3 + 1 ] = 255;
					g_render_info.rgb_data[ y*g_render_info.width*3+x*3 + 2 ] = 255;
				}
			}

		g_render_info.unlock();
	}
}

void streaming_loop()
{
	bool condition = true;
	while( condition )
	{
		g_render_info.lock();
	/*
		//write_jpeg_to_file( g_image_stream.width, g_image_stream.height, &g_image_stream.rgb_data[0] );
	*/
		std::string image_data;
		write_jpeg_to_memory( g_render_info.width, g_render_info.height, &g_render_info.rgb_data[0], image_data );
		g_render_info.unlock();
		WebsocketServer::sendData( "test", image_data );

	};
}

void on_message( const std::string& id, const std::string& message )
{
	std::cout << "on_message!  " << message << std::endl;std::flush(std::cout);

	// parse json ---
	jsmn_parser parser;
	jsmn_init(&parser);
	jsmntok_t tokens[256];
	int r = jsmn_parse(&parser, &message[0], message.size(), tokens, 256);

	/*
	std::cout << "message=" << message << std::endl;
	for( int i=0;i<r;++i )
	{
		std::cout << jsmn_typename(tokens[i].type) << " " << tokens[i].start << "-" << tokens[i].end << " " << tokens[i].size << std::endl;
	}
	*/

	std::map<std::string, jsmntok_t*> map;
	jsmn_to_map( message, &tokens[0], map );

	std::string command = jsmn_to_string(message, map["command"]);
	std::cout << "command=" << command << std::endl;


	if(command == "set")
	{
		std::map<std::string, jsmntok_t*> props;
		jsmn_to_map( message, map["properties"], props );
		for( auto& it:props )
		{
			std::string prop = it.first;
			std::cout << "setting property:" << prop << "=" << jsmn_to_number<double>(message, it.second) << std::endl;
		}

		g_render_info.lock();
		g_render_info.cx = jsmn_to_number<double>(message, props["cx"]);
		g_render_info.cy = jsmn_to_number<double>(message, props["cy"]);
		g_render_info.unlock();
	}

}


int main(int argc, char *argv[])
{

	/*
	{
		std::string message = "{\"command\":\"set\",\"properties\":{\"cx\":0.5,\"cy\":0.5}}";
		jsmn_parser parser;
		jsmn_init(&parser);
		jsmntok_t tokens[256];
		int r = jsmn_parse(&parser, &message[0], message.size(), tokens, 256);
		//std::cout << r << std::endl;

		std::cout << "message=" << message << std::endl;
		for( int i=0;i<r;++i )
		{
			std::cout << jsmn_typename(tokens[i].type) << " " << tokens[i].start << "-" << tokens[i].end << " " << tokens[i].size << std::endl;
		}

		std::map<std::string, jsmntok_t*> map;
		jsmn_to_map( message, &tokens[0], map );

		std::string command = jsmn_to_string(message, map["command"]);
		std::cout << "command=" << command << std::endl;
		if(command == "set")
		{
			std::map<std::string, jsmntok_t*> props;
			jsmn_to_map( message, map["properties"], props );
			for( auto& it:props )
			{
				std::string prop = it.first;
				std::cout << "setting property:" << prop << "=" << jsmn_to_number<double>(message, it.second) << std::endl;
			}
		}

		return 0;
	}
	*/




	QApplication app(argc, argv);


	WebsocketServer::init();
	WebsocketServer::setMessageCallback( on_message );

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

