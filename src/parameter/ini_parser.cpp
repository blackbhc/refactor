#ifndef GALOTFA_INI_PARSER_CPP
#define GALOTFA_INI_PARSER_CPP
#include "ini_parser.h"
#include "../tools/prompt.h"
#ifdef DO_UNIT_TEST
#include "../tools/string.cpp"
#endif
#include "string.h"
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>

namespace galotfa {
ini_parser::ini_parser( const char* file_name )
{
    this->filename = file_name;
#ifndef debug_parameter  // no initialization in debug mode
    this->read( this->filename.c_str() );
#endif
}

void ini_parser::read( const char* file )
{
    struct stat* st = new struct stat;
    stat( file, st );
    this->check_filesize( st->st_size );

    // read the file into one buffer
    FILE* fp = fopen( file, "rb" );
    if ( fp == nullptr )
    {
        delete st;
        st = nullptr;
        ERROR( "The file %s does not exist!\n", file );
    }
    char buffer[ st->st_size + 1 ];
    buffer[ st->st_size ] = '\0';
    char* p_buffer        = buffer;
    fread( p_buffer, ( size_t )st->st_size, 1, fp );

    // parse the buffer
    char* token = strsep( &p_buffer, "\n" );  // get the first line
    while ( token != nullptr )
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
    std::string section_name = "";
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
        ERROR( "The ini file is empty, most common case: the file was not found." );
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
    else if ( raw_content.find_first_of( SECTION_PREFIX ) == 0 )
    // section header
    {
        if ( raw_content.find_first_of( SECTION_SUFFIX ) == raw_content.size() - 1 )
        {
            line.type = ini::LineType::section;
            raw_content.erase( 0, raw_content.find_first_of( SECTION_PREFIX ) + 1 );
            raw_content.erase( raw_content.find_first_of( SECTION_SUFFIX ) );
            std::replace( raw_content.begin(), raw_content.end(), ' ', '_' );
            std::replace( raw_content.begin(), raw_content.end(), '\t', '_' );
            line.content = raw_content;
            return line;
        }
        else
        {
        }
        ERROR( "Get an invalid section header in ini file:\n%s", str );
    }
    else if ( raw_content.find_first_of( KEY_VALUE_SEP ) != std::string::npos )
    // key-value pair
    {
        line.type = ini::LineType::key_value;
        std::string key =
            this->trim( raw_content.substr( 0, raw_content.find_first_of( KEY_VALUE_SEP ) ) );
        std::string val =
            this->trim( raw_content.substr( raw_content.find_first_of( KEY_VALUE_SEP ) + 1 ) );
        if ( val.size() == 0 || key.size() == 0
             || key.find_first_of( VALUE_SEP ) != std::string::npos )
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
            line.content    = key + "=" + "true";
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
                if ( values.size() == 1 )
                {
                    line.value_type = ini::ValueType::number;
                }
                else
                    line.value_type = ini::ValueType::numbers;
            }
            catch ( ... )
            {
                if ( values.size() == 1 )
                {
                    line.value_type = ini::ValueType::string;
                }
                else
                    line.value_type = ini::ValueType::strings;
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


// macro: get the hash key name, and make sure there is such key
// it's only  used in the get functions
#ifndef debug_parameter
#define SECURE_EXTRACT( section, key )                                                    \
    std::string sec_name = section;                                                       \
    std::string key_name = key;                                                           \
    std::replace( sec_name.begin(), sec_name.end(), ' ', '_' );                           \
    std::replace( sec_name.begin(), sec_name.end(), '\t', '_' );                          \
    std::transform( key_name.begin(), key_name.end(), key_name.begin(), ::tolower );      \
    std::string hash_key_name = sec_name + "_" + key_name;                                \
    if ( this->ini_table.find( hash_key_name ) == this->ini_table.end() )                 \
        WARN( "[%s] -> [%s] does not exist in the ini parameter table!", section.c_str(), \
              key.c_str() );
#else
#define SECURE_EXTRACT( section, key )                                                     \
    std::string sec_name = section;                                                        \
    std::string key_name = key;                                                            \
    std::replace( sec_name.begin(), sec_name.end(), ' ', '_' );                            \
    std::replace( sec_name.begin(), sec_name.end(), '\t', '_' );                           \
    std::transform( key_name.begin(), key_name.end(), key_name.begin(), ::tolower );       \
    std::string hash_key_name = sec_name + "_" + key_name;                                 \
    if ( this->ini_table.find( hash_key_name ) == this->ini_table.end() )                  \
        ERROR( "[%s] -> [%s] does not exist in the ini parameter table!", section.c_str(), \
               key.c_str() );
#endif
bool ini_parser::get_bool( std::string section, std::string key ) const
{
    SECURE_EXTRACT( section, key );
    if ( this->ini_table[ hash_key_name ].type != ini::ValueType::boolean )
        WARN( "[%s] -> [%s] is not boolean type!", section.c_str(), key.c_str() );
    return this->ini_table[ hash_key_name ].content == "true";
}

double ini_parser::get_double( std::string section, std::string key ) const
{
    SECURE_EXTRACT( section, key );
    if ( this->ini_table[ hash_key_name ].type != ini::ValueType::number )
        WARN( "[%s] -> [%s] is not number type!", section.c_str(), key.c_str() );
    return std::stod( this->ini_table[ hash_key_name ].content );
}

std::vector< double > ini_parser::get_doubles( std::string section, std::string key ) const
{
    SECURE_EXTRACT( section, key );
    if ( this->ini_table[ hash_key_name ].type != ini::ValueType::numbers )
        WARN( "[%s] -> [%s] is not numbers type!", section.c_str(), key.c_str() );
    auto                  vals = this->split( this->ini_table[ hash_key_name ].content );
    std::vector< double > res( vals.size() );
    for ( size_t i = 0; i < vals.size(); ++i )
    {
        res[ i ] = std::stod( vals[ i ] );
    }
    return res;
}

int ini_parser::get_int( std::string section, std::string key ) const
{
    SECURE_EXTRACT( section, key );
    if ( this->ini_table[ hash_key_name ].type != ini::ValueType::number )
        WARN( "[%s] -> [%s] is not number type!", section.c_str(), key.c_str() );
    return std::stoi( this->ini_table[ hash_key_name ].content );
}

std::vector< int > ini_parser::get_ints( std::string section, std::string key ) const
{
    SECURE_EXTRACT( section, key );
    if ( this->ini_table[ hash_key_name ].type != ini::ValueType::numbers )
        WARN( "[%s] -> [%s] is not numbers type!", section.c_str(), key.c_str() );
    auto               vals = this->split( this->ini_table[ hash_key_name ].content );
    std::vector< int > res( vals.size() );
    for ( size_t i = 0; i < vals.size(); ++i )
    {
        res[ i ] = std::stoi( vals[ i ] );
    }
    return res;
}


std::string ini_parser::get_str( std::string section, std::string key ) const
{
    SECURE_EXTRACT( section, key );
    if ( this->ini_table[ hash_key_name ].type != ini::ValueType::string )
        WARN( "[%s] -> [%s] is not string type!", section.c_str(), key.c_str() );
    return this->ini_table[ hash_key_name ].content;
}

std::vector< std::string > ini_parser::get_strs( std::string section, std::string key ) const
{
    SECURE_EXTRACT( section, key );
    if ( this->ini_table[ hash_key_name ].type != ini::ValueType::strings )
        WARN( "[%s] -> [%s] is not strings type!", section.c_str(), key.c_str() );
    return this->split( this->ini_table[ hash_key_name ].content );
}

bool ini_parser::has( std::string section, std::string key ) const
{
    std::replace( section.begin(), section.end(), ' ', '_' );
    std::replace( section.begin(), section.end(), '\t', '_' );
    std::transform( key.begin(), key.end(), key.begin(), ::tolower );
    std::string hash_key_name = section + "_" + key;
    return this->ini_table.find( hash_key_name ) != this->ini_table.end();
}

#ifdef debug_parameter

int ini_parser::test_checksize() const
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

bool ini_parser::check_line_equal( ini::Line a, ini::Line b ) const
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

int ini_parser::test_lineparser( void ) const
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
    char bad_key_value1[]    = " = ";
    char bad_key_value2[]    = "1 = ";
    char bad_key_value3[]    = " = 12";
    char bad_key_value4[]    = "key1 key2 = 12";

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
    expected_section_line_res.content       = "This_is_a_section";
    expected_section_line_res.type          = ini::LineType::section;
    expected_section_line_res.value_type    = ini::ValueType::none;
    expected_key_number_line_res.content    = "key_char=1.0 12";
    expected_key_number_line_res.type       = ini::LineType::key_value;
    expected_key_number_line_res.value_type = ini::ValueType::numbers;
    expected_key_bool_line_res.content      = "key_bool=true";
    expected_key_bool_line_res.type         = ini::LineType::key_value;
    expected_key_bool_line_res.value_type   = ini::ValueType::boolean;
    expected_key_str_line_res.content       = "key_str=1 true";
    expected_key_str_line_res.type          = ini::LineType::key_value;
    expected_key_str_line_res.value_type    = ini::ValueType::strings;

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

    CHECK_RETURN( success );
}

int ini_parser::test_trim( void ) const
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

    CHECK_RETURN( success );
}

int ini_parser::test_split( void ) const
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
    CHECK_RETURN( success );
}

int ini_parser::test_read( void )
{
    println( "Testing ini_parser::read(const char* filename) ..." );
    try
    {
        this->read( this->filename.c_str() );
        println( "It passed the test." );
        return 0;
    }
    catch ( std::exception& e )
    {
        WARN( "It failed the test, error message: %s", e.what() );
        return 1;
    }
}

int ini_parser::test_get( void ) const
{
    println( "Testing ini_parser::get<T> ..." );

    // insert some test data to the hash table
    ini::Value number, numbers, string, strings, boolean;
    number.content  = "3.1415926";
    number.type     = ini::ValueType::number;
    numbers.content = "1,2,3,4";
    numbers.type    = ini::ValueType::numbers;
    string.content  = "This_is_a_string";
    string.type     = ini::ValueType::string;
    strings.content = "1,2,3,4";
    strings.type    = ini::ValueType::strings;
    boolean.content = "true";
    boolean.type    = ini::ValueType::boolean;

    ini_table[ "TestSec_number" ]  = number;
    ini_table[ "TestSec_numbers" ] = numbers;
    ini_table[ "TestSec_string" ]  = string;
    ini_table[ "TestSec_strings" ] = strings;
    ini_table[ "TestSec_boolean" ] = boolean;

    // test the get function
    bool   target1 = true;
    double target2 = 3.1415926;
    auto   target3 = std::vector< double >{ 1, 2, 3, 4 };
    auto   target4 = std::vector< std::string >{ "1", "2", "3", "4" };
    auto   target5 = "This_is_a_string";

    auto res1 = get_bool( "TestSec", "boolean" );
    auto res2 = get_double( "TestSec", "number" );
    auto res3 = get_doubles( "TestSec", "numbers" );
    auto res4 = get_strs( "TestSec", "strings" );
    auto res5 = get_str( "TestSec", "string" );

    // erase the test data
    ini_table.erase( "TestSec_boolean" );
    ini_table.erase( "TestSec_number" );
    ini_table.erase( "TestSec_numbers" );
    ini_table.erase( "TestSec_string" );
    ini_table.erase( "TestSec_strings" );

    println( "Test the case of non-exist key:" );
    try
    {
        get_bool( "TestSec", "non_exist_key" );
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

    bool success = ( res1 == target1 ) && ( res2 - target2 < 1e-10 ) && ( res3 == target3 )
                   && ( res4 == target4 ) && ( res5 == target5 );
    CHECK_RETURN( success );
    return 0;
}
#endif
}  // namespace galotfa
#endif
