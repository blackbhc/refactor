/// This file is a naive implementation of a parser for ini files.
/// Only supports simple key-value pairs: characters and numbers only.

#ifndef __INI_PARSER_H__
#define __INI_PARSER_H__
#include <string>
#include <vector>
namespace galotfa {
class ini_parser
{
private:  // private members
    enum class ValueType { number, string, boolean, none };
    enum class LineType { section, key_value, empty };
    struct Line
    {
        std::string content    = "";
        LineType    type       = LineType::empty;
        ValueType   value_type = ValueType::none;
    };
    // define the delimiters
    static const std::string blank;
    static const std::string comment_prefix;
    static const std::string section_prefix;
    static const std::string section_suffix;
    static const std::string key_value_sep;
    static const std::string value_sep;
    std::vector< Line >      lines;

public:  // public members
    std::string filename;

public:  // public methods
    ini_parser( const char* filename );
    ~ini_parser() = default;
#ifdef debug_parameter
    int  test_checksize();
    int  test_trim();
    int  test_split();
    int  test_lineparser();
    int  test_read();
    bool check_line_equal( Line a, Line b );
#endif

private:  // private methods
    void                              read( const char* filename );
    Line                              line_parser( const char* str ) const;
    void                              check_filesize( long int size ) const;
    inline std::string                trim( std::string str ) const;
    inline std::vector< std::string > split( std::string str ) const;
};

}  // namespace galotfa

#endif
