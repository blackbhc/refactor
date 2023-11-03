/// This file is a naive implementation of a parser for ini files.
/// Only supports simple key-value pairs: characters and numbers only.

#ifndef GALOTFA_INI_PARSER_H
#define GALOTFA_INI_PARSER_H
#include "../tools/string.h"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#define BLANK " \t\n\r\f\v"
#define COMMENT_PREFIX "#;"
#define SECTION_PREFIX "["
#define SECTION_SUFFIX "]"
#define KEY_VALUE_SEP "="
#define VALUE_SEP " \t,:+&"

namespace galotfa {
namespace ini {
    enum class ValueType { number, numbers, string, strings, boolean, none };
    enum class LineType { section, key_value, empty };
    struct Line
    {
        std::string content    = "";
        LineType    type       = LineType::empty;
        ValueType   value_type = ValueType::none;
    };
    struct Value
    {
        std::string content = "";
        ValueType   type    = ValueType::none;
    };
}  // namespace ini
class ini_parser
{
private:  // private members
    mutable std::unordered_map< std::string, ini::Value > ini_table;
    mutable std::string* _section_name;  // a tmp variable which is used to store the section name
                                         // in insert_to_table()

public:  // public members
    std::string filename;

public:  // public methods
    ini_parser( const char* filename );
    ~ini_parser()
    {
        delete _section_name;
    }
    bool                       get_bool( std::string section, std::string key ) const;
    double                     get_double( std::string section, std::string key ) const;
    std::vector< double >      get_doubles( std::string section, std::string key ) const;
    int                        get_int( std::string section, std::string key ) const;
    std::vector< int >         get_ints( std::string section, std::string key ) const;
    std::string                get_str( std::string section, std::string key ) const;
    std::vector< std::string > get_strs( std::string section, std::string key ) const;
    bool                       has( std::string section, std::string key ) const;
#ifdef debug_parameter
    int  test_checksize() const;
    int  test_trim() const;
    int  test_split() const;
    int  test_lineparser() const;
    int  test_read();
    bool check_line_equal( ini::Line a, ini::Line b ) const;
    int  test_get( void ) const;
#endif

private:  // private methods
    void               read( const char* filename );
    ini::Line          line_parser( const char* str ) const;
    void               check_filesize( long int size ) const;
    inline std::string trim( std::string str ) const
    {
        str = galotfa::string::trim( str, BLANK );
        if ( str.find_first_of( COMMENT_PREFIX ) != std::string::npos )
        {
            str.erase( str.find_first_of( COMMENT_PREFIX ) );
        }
        return str;
    }

    inline std::vector< std::string > split( std::string str ) const
    {
        return galotfa::string::split( str, VALUE_SEP );
    }

    void insert_to_table( ini::Line                                      line,
                          std::unordered_map< std::string, ini::Value >& hash ) const;
};

}  // namespace galotfa

#endif
