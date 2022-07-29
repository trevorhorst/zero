#include "common/types.h"

namespace Types {

/**
 * @brief Character array comparator
 * @param a First comparator
 * @param b Second comparator
 * @return bool returns true if the strings are equal
 */
bool CharArrayComparator::operator()( const char *a, const char *b ) const
{
    return strcmp( a, b ) < 0;
}

/**
 * @brief DJB2 hash algorithm
 * @param str Char array to hash
 * @return Integer representation of the hash value
 */
u_long CharArrayHash::operator()( const char *str ) const
{
    u_long hash = 5381;
    int c = 0;

    while( ( c = *str++ ) ) {
        /* hash * 33 + c */
        hash = ( ( hash << 5 ) + hash ) + static_cast< u_long >( c );
    }

    return hash;
}

/**
 * @brief Character array hash comparator
 * @param a First comparator
 * @param b Second comparator
 * @return bool returns true if the strings are equal
 */
bool CharArrayHashComparator::operator()( const char *a, const char *b ) const
{
    return !strcmp( a, b );
}

}
