/// This file is a naive implementation of a parser for ini files.
/// Only supports simple key-value pairs: characters and numbers only.

#ifndef __INI_PARSER_H__
#define __INI_PARSER_H__
#include <string>
namespace galotfa {
class ini_parser
{
    // public methods
public:
    ini_parser( const char* filename );
    ~ini_parser() = default;
#ifdef debug_parameter
    int test_check_size_works();
#endif
    // private methods
private:
    void read( const char* filename );
    void check_filesize( long int size );

    // public members
public:
    std::string filename;
    // private members
};

}  // namespace galotfa

namespace ini {
enum class Valuetype { character, number, string, boolean, none };
enum class Linetype { comment, section, key_value, white };
}  // namespace ini

#endif
