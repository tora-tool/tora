#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

std::string global_string = "static const unsigned char tora_toad[] =\n\"";

inline std::string stringify(unsigned char x)
{
    std::string s;
    std::ostringstream o;
    o << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(x);
    s = o.str();
   return s;
 }

std::string readBinaryFile(const std::string& filename)
{
    std::ifstream input(filename.c_str(), std::ios::in | std::ios::binary);
    char c;
    std::string tmp, rc;

    while (input.get(c)) {
	tmp = "\\x";
	tmp += stringify(static_cast<unsigned char>(c));
	rc.append(tmp);
    }
    return rc;
}


int main(int argc, char** argv)
{
    std::string s = readBinaryFile(argv[1]);
    
    for (size_t i=0; i < s.length(); ++i){
	if ( ( i>0 ) && ( i%160 == 0 ) )
	    global_string.append("\"\n\"");
	global_string.push_back(s[i]);
    }
    global_string.append("\"\n;\n");
    std::cout << global_string;
}
