#include <iostream>

/*
#include <QApplication>
#include <WebsocketServer.h>
#include <RenderServer.h>

#include <jsmn/jsmn_util.h>






struct DummyRenderer : public ProgressiveRendererInterface
{
	DummyRenderer():
		ProgressiveRendererInterface()
	{
		width = 512;
		height = 512;
		rgb_data = std::vector<float>( width*height*3, 0 );

		t = 0.0;
		cx = 0.5 + 0.3*std::cos(t);
		cy = 0.5 - 0.3*std::sin(t);
	}

	int width;
	int height;
	std::vector<float> rgb_data;


	double t;
	double cx, cy;


	virtual void advance() override
	{
		std::fill(rgb_data.begin(), rgb_data.end(), 0);

		t += 0.1;

		// do some shit
		for( int y=0;y<height;++y )
			for( int x=0;x<width;++x )
			{
				//double u = double(i)/double(g_render_info.width-1);
				//double v = double(j)/double(g_render_info.height-1);

				//double x = u-g_render_info.cx;
				//double y = v-g_render_info.cy;
				double x2 = x-cx;
				double y2 = y-cy;
				double d = std::sqrt( x2*x2+y2*y2 );
				if( d < 20.0 )
				{
					rgb_data[ y*width*3+x*3 + 0 ] = 1.0;
					rgb_data[ y*width*3+x*3 + 1 ] = 1.0;
					rgb_data[ y*width*3+x*3 + 2 ] = 1.0;
				}
			}
	}

	virtual float *getRGBData( int& width, int& height )override
	{
		width = this->width;
		height = this->height;
		return &rgb_data[0];
	}

private:

};











RenderServer* g_rserver=0;








void on_message( const std::string& id, const std::string& message )
{
	std::cout << "on_message!  " << message << std::endl;std::flush(std::cout);

	// parse json ---
	jsmn_parser parser;
	jsmn_init(&parser);
	jsmntok_t tokens[256];
	int r = jsmn_parse(&parser, &message[0], message.size(), tokens, 256);


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

		DummyRenderer* rinfo = dynamic_cast<DummyRenderer*>(g_rserver->lock());
		rinfo->cx = jsmn_to_number<double>(message, props["cx"]);
		rinfo->cy = jsmn_to_number<double>(message, props["cy"]);
		g_rserver->unlock();
	}

}


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	DummyRenderer dummyRenderer;

	RenderServerGUI gui(&dummyRenderer);
	g_rserver = gui.getRServer();
	gui.show();

	WebsocketServer::setMessageCallback( on_message );

	int result = app.exec();
	return result;
}
*/
