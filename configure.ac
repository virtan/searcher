AC_PREREQ(2.59)
AC_INIT(searcher, 0.1, virtan@itim.vn)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([srch/config_branch.h])
AC_CONFIG_HEADER([srch/config.h])
AC_CONFIG_MACRO_DIR([m4])

# Define _GNU_SOURCE
AC_GNU_SOURCE

AM_MAINTAINER_MODE([])

# Dynamic only
#AC_DISABLE_STATIC
AC_DISABLE_SHARED

# libtool support
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Lang for tests
AC_LANG(C++)
CXXFLAGS="$CXXFLAGS -std=c++0x -W -Wall -Werror -pthread"

# Checks for programs.
AC_PROG_CXX([g++-4.8 g++-4.7 g++-4.6 g++-4.5 g++ c++ cc cxx])
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S

# Protobuf check
AC_CHECK_PROG(PROTOC, protoc, protoc)
if @<:@ ! "${PROTOC}" @:>@ ; then
    echo "Protobuf needed to build"
    exit 1
fi

# Check for unordered_map
AH_TEMPLATE([unordered_map_reserve],[Presence of unordered_map::reserve])
AC_COMPILE_IFELSE([
AC_LANG_PROGRAM([
#include <unordered_map>],
[std::unordered_map<int,int> um;
um.reserve(10);])],
[AC_DEFINE([unordered_map_reserve])],[])

# Options
AC_SYS_LARGEFILE

# Check for ICU
AC_CHECK_ICU(4.4, [], [AC_MSG_ERROR([ICU 4.4+ needed])])

# Check for boost
AX_BOOST_BASE([1.48],[],[AC_MSG_ERROR([Boost 1.48+ needed])])
AX_BOOST_THREAD
AX_BOOST_LOCALE
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_FILESYSTEM

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([boost/optional.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/property_tree/json_parser.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/exception/all.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/array.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/assign/list_of.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/range/iterator_range.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/interprocess/mapped_region.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/shared_ptr.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([boost/scoped_ptr.hpp],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([algorithm],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([fstream],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([iostream],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([iterator],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([stddef.h],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([stdexcept],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([stdint.h],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([string],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([sys/mman.h],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([unordered_map],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([vector],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([google/protobuf/message.h],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([google/protobuf/repeated_field.h],[],[AC_MSG_ERROR(Header absent)])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_TYPE_UINT32_T

AC_CACHE_SAVE

# Logger
AC_CHECK_HEADERS([eger/logger.h],[],[
    AC_MSG_NOTICE([no external logger found, building internal])
    AC_CONFIG_SUBDIRS(eger)
    BUILD_EGER=eger
    AC_SUBST([LOGGER_CPPFLAGS], ["-I`pwd`/eger"])
    AC_SUBST([LOGGER_LIBS], ["-L`pwd`/eger/eger -leger"])
])
AC_SUBST([BUILD_EGER])

AC_CACHE_SAVE

AC_CONFIG_FILES([
        Makefile
        srch/Makefile
        tests/Makefile
])

AC_OUTPUT

for cfg_hdr in $ac_config_headers :; do
    echo "filtering $cfg_hdr"
    if @<:@ -f $cfg_hdr @:>@ ; then
        sed -i "s/^\(#define PACKAGE.*\)$/\/* \\1 *\//g" $cfg_hdr
        grep "^#define" $cfg_hdr | cut -d ' ' -f 1,2  | sed "s/#define/#undef/g"  > ${cfg_hdr}_undef
        cat ${cfg_hdr}_undef $cfg_hdr > ${cfg_hdr}_new
        mv ${cfg_hdr}_new $cfg_hdr
        rm -f ${cfg_hdr}_undef
    fi
done
