#include "ini-parser.h"
#include "../tools/prompt.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

namespace galotfa {

const std::string ini_parser::blank          = " \t\n\r\f\v";
const std::string ini_parser::comment_prefix = "#;";
const std::string ini_parser::section_prefix = "[";
const std::string ini_parser::section_suffix = "]";
const std::string ini_parser::key_value_sep  = "=";
const std::string ini_parser::value_sep      = " \t,-:+&";

ini_parser::ini_parser( const char* file_name )
{
    this->filename = file_name;
#ifndef debug_parameter  // no initialization in debug mode
    this->read( this->filename.c_str() );
#endif
}

void ini_parser::read( const char* filename )
{
    struct stat* st = new struct stat;
    stat( filename, st );
    this->check_filesize( st->st_size );

    // read the file into one buffer
    FILE* fp = fopen( filename, "rb" );
    if ( fp == NULL )
    {
        delete st;
        st = nullptr;
        ERROR( "The file %s does not exist!\n", filename );
    }
    char buffer[ st->st_size + 1 ];
    buffer[ st->st_size ] = '\0';
    char* p_buffer        = buffer;
    fread( p_buffer, st->st_size, 1, fp );

    // parse the buffer
    char* token = strsep( &p_buffer, "\n" );  // get the first line
    while ( token != NULL )
    {
        if ( *token == '\0' )
        {
        }
        else
        {
            // TODO: call the function to inter the line into the hash table
            this->insert_to_table( this->line_parser( token ), this->ini_table );
        }
        token = strsep( &p_buffer, "\n" );
    }

    fclose( fp );
    delete st;
    st = nullptr;
}

void ini_parser::insert_to_table( ini::Line                                      line,
                                  std::unordered_map< std::string, ini::Value >& hash ) const
// remain the second parameter for convenience of unit test
{
    static std::string section_name = "";
    switch ( line.type )
    {
    case ini::LineType::empty:
        break;
    case ini::LineType::section:
        section_name = line.content;
        break;
    case ini::LineType::key_value:
        size_t     pos = line.content.find_first_of( "=" );
        ini::Value value;
        value.content                                              = line.content.substr( pos + 1 );
        value.type                                                 = line.value_type;
        hash[ section_name + "_" + line.content.substr( 0, pos ) ] = value;
        break;
    }
}

void ini_parser::check_filesize( long int size ) const
{
    if ( size == 0 )
    {
        ERROR( "The ini file is empty." );
    }
    else if ( size >= 1024 * 1024 * 5 && size < 1024 * 1024 * 1024 )
    {
        WARN( "The ini file is too big: size = %ld MB.", size / 1024 / 1024 );
    }
    else if ( size >= 1024 * 1024 * 1024 )
    {
        ERROR( " The ini file is too big: size = %ld MB\nPlease check whether the your file is "
               "correct!",
               size / 1024 / 1024 );
    }
}

ini::Line ini_parser::line_parser( const char* str ) const
{
    ini::Line   line;
    std::string raw_content = str;
    raw_content             = this->trim( raw_content );

    // check whether it is a section
    if ( raw_content.size() == 0 )  // empty line
    {
        return line;
        // with default types: LineType::empty and ValueType::none
    }
    else if ( raw_content.find_first_of( section_prefix ) == 0 )
    // section header
    {
        if ( raw_content.find_first_of( section_suffix ) == raw_content.size() - 1 )
        {
            line.type = ini::LineType::section;
            raw_content.erase( 0, raw_content.find_first_of( section_prefix ) + 1 );
            raw_content.erase( raw_content.find_first_of( section_suffix ) );
            std::replace( raw_content.begin(), raw_content.end(), ' ', '_' );
            line.content = raw_content;
            return line;
        }
        else
        {
        }
        ERROR( "Get an invalid section header in ini file:\n%s", str );
    }
    else if ( raw_content.find_first_of( key_value_sep ) != std::string::npos )
    // key-value pair
    {
        line.type = ini::LineType::key_value;
        std::string key =
            this->trim( raw_content.substr( 0, raw_content.find_first_of( key_value_sep ) ) );
        std::string val =
            this->trim( raw_content.substr( raw_content.find_first_of( key_value_sep ) + 1 ) );
        if ( val.size() == 0 || key.size() == 0
             || key.find_first_of( value_sep ) != std::string::npos )
        {
            ERROR( "Get an invalid line in ini file:\n%s", str );
        }
        line.content = key + "=" + val;

        // determine the value type
        std::transform( val.begin(), val.end(), val.begin(), ::tolower );
        if ( val == "true" || val == "yes" || val == "enable" || val == "on" || val == "false"
             || val == "no" || val == "disable" || val == "off" )
        {
            line.value_type = ini::ValueType::boolean;
            return line;
        }
        else
        {
            auto values = this->split( val );
            try
            {
                for ( auto& v : values )
                {
                    std::stod( v );
                }
                line.value_type = ini::ValueType::number;
            }
            catch ( ... )
            {
                line.value_type = ini::ValueType::string;
            }
            return line;
        }
    }
    else
    {
        ERROR( "Get an invalid line in ini file:\n%s", str );
    }
    WARN( "Encounter an return of empty line." );
    return line;
}

inline std::string ini_parser::trim( std::string str ) const
{
    str.erase( 0, str.find_first_not_of( blank ) );
    str.erase( str.find_last_not_of( blank ) + 1 );
    // remove the possible comments
    if ( str.find_first_of( comment_prefix ) != std::string::npos )
    {
        str.erase( str.find_first_of( comment_prefix ) );
    }
    return str;
}

inline std::vector< std::string > ini_parser::split( std::string str ) const
{
    std::vector< std::string > vals;
    str = this->trim( str );
    while ( true )
    {
        size_t pos = str.find_first_of( value_sep );
        vals.push_back( str.substr( 0, pos ) );
        str.erase( 0, pos );
        str.erase( 0, str.find_first_not_of( value_sep ) );
        if ( str.find_first_of( value_sep ) == std::string::npos )
        {
            vals.push_back( str );
            break;
        }
    }
    return vals;
}

#ifdef debug_parameter
int ini_parser::test_checksize()
{
    println( "Testing ini_parser::check_filesize() ..." );
    try
    {
        println( "Check whethet it can distinguish a file with size 0:" );
        try
        {
            check_filesize( 0 );
        }
        catch ( std::runtime_error& e )
        {
            println( "It failed as expected, error message: %s", e.what() );
        }
        catch ( ... )
        {
            println( "It failed unexpectedly!" );
            return 1;
        }

        try
        {
            println( "Check whethet it can distinguish a file with size 1GB:" );
            check_filesize( 1024 * 1024 * 1024 );
        }
        catch ( const std::runtime_error& e )
        {
            println( "It failed as expected, error message: %s", e.what() );
        }
        catch ( ... )
        {
            println( "It failed unexpectedly!\n" );
            return 1;
        }

        try
        {
            println( "Check whethet it can distinguish a file with size 5MB:" );
            check_filesize( 1024 * 1024 * 5 );
        }
        catch ( const std::runtime_error& e )
        {
            println( "It failed as expected, error message: %s", e.what() );
        }
        catch ( ... )
        {
            println( "It failed unexpectedly!\n" );
            return 1;
        }

        return 0;
    }
    catch ( const std::exception& e )
    {
        println( "It failed unexpectedly, error message: %s", e.what() );
        return 1;
    }
}

bool ini_parser::check_line_equal( ini::Line a, ini::Line b )
{
    return ( a.content == b.content ) && ( a.type == b.type ) && ( a.value_type == b.value_type );
}

// internal use only: the make the code more compact
#define compact_try_catch( sentence )                                        \
    {                                                                        \
        try                                                                  \
        {                                                                    \
            sentence;                                                        \
        }                                                                    \
        catch ( std::runtime_error & e )                                     \
        {                                                                    \
            println( "It failed as expected, error message: %s", e.what() ); \
        }                                                                    \
        catch ( ... )                                                        \
        {                                                                    \
            println( "It failed unexpectedly!" );                            \
            return 1;                                                        \
        }                                                                    \
    }

int ini_parser::test_lineparser( void )
{
    println( "Testing ini_parser::line_parser(const char* str) ..." );

    // legal lines
    char empty_line[]      = "";
    char blank_line[]      = "\t  \n";
    char comment_line[]    = "# this is a comment";
    char section_line[]    = "[This is a section]";
    char key_number_line[] = "key_char = 1.0 12";
    char key_bool_line[]   = "key_bool = true";
    char key_str_line[]    = "key_str = 1 true";
    // illegal lines
    char bad_section_line1[] = "[This is a bad section";
    char bad_section_line2[] = "+[This is a bad section]";
    char bad_key_value1[]    = " = \n";
    char bad_key_value2[]    = "1 = \n";
    char bad_key_value3[]    = " = 12\n";
    char bad_key_value4[]    = "key1 key2 = 12\n";

    auto empty_line_res      = line_parser( empty_line );
    auto blank_line_res      = line_parser( blank_line );
    auto comment_line_res    = line_parser( comment_line );
    auto section_line_res    = line_parser( section_line );
    auto key_number_line_res = line_parser( key_number_line );
    auto key_bool_line_res   = line_parser( key_bool_line );
    auto key_str_line_res    = line_parser( key_str_line );

    ini::Line expected_empty_line_res, expected_blank_line_res, expected_comment_line_res,
        expected_section_line_res, expected_key_number_line_res, expected_key_bool_line_res,
        expected_key_str_line_res;
    expected_empty_line_res.content         = "";
    expected_empty_line_res.type            = ini::LineType::empty;
    expected_empty_line_res.value_type      = ini::ValueType::none;
    expected_blank_line_res.content         = "";
    expected_blank_line_res.type            = ini::LineType::empty;
    expected_blank_line_res.value_type      = ini::ValueType::none;
    expected_comment_line_res.content       = "";
    expected_comment_line_res.type          = ini::LineType::empty;
    expected_comment_line_res.value_type    = ini::ValueType::none;
    expected_section_line_res.content       = "This is a section";
    expected_section_line_res.type          = ini::LineType::section;
    expected_section_line_res.value_type    = ini::ValueType::none;
    expected_key_number_line_res.content    = "key_char=1.0 12";
    expected_key_number_line_res.type       = ini::LineType::key_value;
    expected_key_number_line_res.value_type = ini::ValueType::number;
    expected_key_bool_line_res.content      = "key_bool=true";
    expected_key_bool_line_res.type         = ini::LineType::key_value;
    expected_key_bool_line_res.value_type   = ini::ValueType::boolean;
    expected_key_str_line_res.content       = "key_str=1 true";
    expected_key_str_line_res.type          = ini::LineType::key_value;
    expected_key_str_line_res.value_type    = ini::ValueType::string;

    compact_try_catch( auto bad_section_line1_res = line_parser( bad_section_line1 ) );
    compact_try_catch( auto bad_section_line2_res = line_parser( bad_section_line2 ) );
    compact_try_catch( auto bad_key_value1_res = line_parser( bad_key_value1 ) );
    compact_try_catch( auto bad_key_value2_res = line_parser( bad_key_value2 ) );
    compact_try_catch( auto bad_key_value3_res = line_parser( bad_key_value3 ) );
    compact_try_catch( auto bad_key_value3_res = line_parser( bad_key_value4 ) );

    bool success = check_line_equal( empty_line_res, expected_empty_line_res )
                   && check_line_equal( blank_line_res, expected_blank_line_res )
                   && check_line_equal( comment_line_res, expected_comment_line_res )
                   && check_line_equal( section_line_res, expected_section_line_res )
                   && check_line_equal( key_number_line_res, expected_key_number_line_res )
                   && check_line_equal( key_bool_line_res, expected_key_bool_line_res )
                   && check_line_equal( key_str_line_res, expected_key_str_line_res );

    if ( success )
    {
        println( "It passed the test." );
        return 0;
    }
    else
    {
        WARN( "It failed the test." );
        return 1;
    }
}

int ini_parser::test_trim( void )
{
    println( "Testing ini_parser::trim(std::string str) ..." );

    char str1[] = "  \t  \n  \r  \f  \v  OK";
    char str2[] = "  \t  \n  \r  \f  \v  # test the comment";
    char str3[] = "# This is a comment \n\n";
    char str4[] = "  \t  \n  \r  \f  \vstr before comment# asdqw";
    char str5[] = " enclosed by space ";
    char str6[] = "1:::-+2;3-4";

    bool res1 = trim( str1 ) == "OK";
    bool res2 = trim( str2 ) == "";
    bool res3 = trim( str3 ) == "";
    bool res4 = trim( str4 ) == "str before comment";
    bool res5 = trim( str5 ) == "enclosed by space";
    bool res6 = trim( str6 ) == "1:::-+2";

    bool success = res1 && res2 && res3 && res4 && res5 && res6;

    if ( success )
    {
        println( "It passed the test." );
        return 0;
    }
    else
    {
        WARN( "It failed the test." );
        return 1;
    }
}

#include <assert.h>
int ini_parser::test_split( void )
{
    println( "Testing ini_parser::split(std::string str) ..." );

    std::string str1 = " 1,2,3 4";
    std::string str2 = "1 2\t 3 \t\t4";
    std::string str3 = "1+2:3:4";
    std::string str4 = "1:::-+2\t3-4";
    std::string str5 = "1.012+string+3&4";

    std::vector< std::string > target1 = { "1", "2", "3", "4" };
    std::vector< std::string > target2 = { "1.012", "string", "3", "4" };

    auto res1 = split( trim( str1 ) );
    auto res2 = split( trim( str2 ) );
    auto res3 = split( trim( str3 ) );
    auto res4 = split( trim( str4 ) );
    auto res5 = split( trim( str5 ) );

    bool success = ( res1 == target1 ) && ( res2 == target1 ) && ( res3 == target1 )
                   && ( res4 == target1 ) && ( res5 == target2 );
    if ( success )
    {
        println( "It passed the test." );
        return 0;
    }
    else
    {
        WARN( "It failed the test." );
        return 1;
    }
}

int ini_parser::test_read( void )
{
    try
    {
        this->read( this->filename.c_str() );
        return 0;
    }
    catch ( std::exception& e )
    {
        WARN( "It failed the test, error message: %s", e.what() );
        return 1;
    }
}

#endif

}  // namespace galotfa
