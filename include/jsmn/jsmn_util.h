#pragma once
#include <string>
#include <cmath>
#include <map>
#include <vector>
#include <jsmn/jsmn.h>


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


std::string jsmn_typename( jsmntype_t type );
std::string jsmn_to_string( const std::string& json, jsmntok_t* t );

// skips jumps to the next key token with an object
int jsmn_next_key( jsmntok_t* key_token );

void jsmn_to_map( const std::string& json, jsmntok_t* t, std::map<std::string, jsmntok_t*>& map );

template<class T>
T jsmn_to_number( const std::string& json, jsmntok_t* t)
{
	std::istringstream stream (json.substr(t->start, t->end-t->start));
	T value;
	stream >> value;
	return value;
}


inline std::string jsmn_trim_right (const std::string & s, const std::string & t = " \t\r\n");

inline std::string jsmn_trim_left (const std::string & s, const std::string & t = " \t\r\n");

inline std::string jsmn_trim (const std::string & s, const std::string & t = " \t\r\n");

std::string jsmn_replace(std::string &s, const std::string& toReplace, const std::string& replaceWith);
// split a line into the first word, and rest-of-the-line
std::string jsmn_get_word (std::string & s, const std::string delim = " ",const bool trim_spaces = true);

void jsmn_split_string( const std::string s, std::vector<std::string> & v, const std::string delim = " ", const bool trim_spaces = true);




// ------------------- string - type conversion  ------------------
template<class T>
inline std::string jsmn_to_string(const T& t)
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}

template<class T>
T jsmn_from_string(const std::string& s)
{
	std::istringstream stream (s);
	T t;
	stream >> t;
	return t;
}





