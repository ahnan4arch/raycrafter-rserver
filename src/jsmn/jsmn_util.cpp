#include <jsmn/jsmn_util.h>








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

