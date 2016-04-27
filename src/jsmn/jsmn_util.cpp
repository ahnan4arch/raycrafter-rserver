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

inline std::string jsmn_trim_right (const std::string & s, const std::string & t)
{
	std::string d (s);
	std::string::size_type i (d.find_last_not_of (t));
	if (i == std::string::npos)
		return "";
	else
		return d.erase (d.find_last_not_of (t) + 1) ;
}  // end of trim_right

inline std::string jsmn_trim_left (const std::string & s, const std::string & t)
{
	std::string d (s);
	return d.erase (0, s.find_first_not_of (t)) ;
}  // end of trim_left

inline std::string jsmn_trim (const std::string & s, const std::string & t)
{
	std::string d (s);
	return jsmn_trim_left (jsmn_trim_right (d, t), t) ;
}  // end of trim


// split a line into the first word, and rest-of-the-line
std::string jsmn_get_word (std::string & s, const std::string delim,const bool trim_spaces)
{
	// find delimiter
	std::string::size_type i (s.find (delim));

	// split into before and after delimiter
	std::string w (s.substr (0, i));

	// if no delimiter, remainder is empty
	if (i == std::string::npos)
		s.erase ();
	else
		// erase up to the delimiter
		s.erase (0, i + delim.size ());

	// trim spaces if required
	if (trim_spaces)
	{
		w = jsmn_trim (w);
		s = jsmn_trim (s);
	}

	// return first word in line
	return w;
} // end of getWord

// To be symmetric, we assume an empty string (after trimming spaces)
// will give an empty vector.
// However, a non-empty string (with no delimiter) will give one item
// After that, you get an item per delimiter, plus 1.
// eg.  ""      => empty
//      "a"     => 1 item
//      "a,b"   => 2 items
//      "a,b,"  => 3 items (last one empty)
void jsmn_split_string( const std::string s, std::vector<std::string> & v, const std::string delim, const bool trim_spaces)
{
	// start with initial string, trimmed of leading/trailing spaces if required
	std::string s1 (trim_spaces ? jsmn_trim (s) : s);

	v.clear (); // ensure vector empty

	// no string? no elements
	if (s1.empty ())
		return;

	// add to vector while we have a delimiter
	while (!s1.empty () && s1.find (delim) != std::string::npos)
		v.push_back (jsmn_get_word (s1, delim, trim_spaces));

	// add final element
	v.push_back (s1);
} // end of splitString


std::string jsmn_replace(std::string &s, const std::string &toReplace, const std::string &replaceWith)
{
	const size_t pos = s.find(toReplace);
	if(pos!=std::string::npos)
		return(s.replace(pos, toReplace.length(), replaceWith));
	else
		return s;
}
