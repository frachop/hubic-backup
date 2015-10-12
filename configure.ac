#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(hubicbackup, version-0.1)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/asset.cpp])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
# AC_PROG_CC
AC_LANG(C++)

AX_CXX_COMPILE_STDCXX_11

# Checks for header files.
AC_CHECK_HEADERS([stdint.h string syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([clock_gettime localtime_r memset strerror])

# Checks for libraries.

# --- CURL -----------------------------------------------------
AC_CHECK_HEADERS([curl/curl.h], [], [AC_MSG_ERROR([Can't find curl headers])])
AC_CHECK_LIB(
	curl, 
	curl_easy_init, 
	[], 
	[AC_MSG_ERROR([Can't find curl library])]
)

# --- CRYPTO ---------------------------------------------------
AC_CHECK_HEADERS([openssl/evp.h openssl/rand.h], [], [AC_MSG_ERROR([Can't find openssl headers])])
AC_CHECK_LIB(
	crypto, 
	[EVP_CIPHER_CTX_init], 
	[], 
	[AC_MSG_ERROR([Can't find crypto library (openssl)])]
)

# --- JSONCPP ---------------------------------------------------
AC_CHECK_HEADERS([jsoncpp/json/json.h], [], [AC_MSG_ERROR([Can't find jsoncpp headers])])
AC_HAVE_LIBRARY(
	jsoncpp, 
	[], 
	[AC_MSG_ERROR([Can't find jsoncpp library])]
)

# --- BOOST-FILESYSTEM ---------------------------------------------------
AC_CHECK_HEADERS([boost/system/api_config.hpp], [], [AC_MSG_ERROR([Can't find boost system headers])])
AC_CHECK_HEADERS([boost/filesystem.hpp], [], [AC_MSG_ERROR([Can't find boost filesystem headers])])
AC_HAVE_LIBRARY(
	boost_system, 
	[], 
	[AC_MSG_ERROR([Can't find boost_system library])]
)

AC_HAVE_LIBRARY(
	boost_filesystem, 
	[], 
	[AC_MSG_ERROR([Can't find boost_filesystem library])]
)

# --- BOOST-REGEX ---------------------------------------------------
AC_CHECK_HEADERS([boost/regex.hpp], [], [AC_MSG_ERROR([Can't find boost regex headers])])
AC_HAVE_LIBRARY(
	boost_regex, 
	[], 
	[AC_MSG_ERROR([Can't find boost_regex library])]
)

# --- BOOST-PROGRAM-OPTIONS ---------------------------------------------------
AC_CHECK_HEADERS([boost/program_options/option.hpp], [], [AC_MSG_ERROR([Can't find boost_program_options headers])])
AC_HAVE_LIBRARY(
	boost_program_options,
	[], 
	[AC_MSG_ERROR([Can't find boost_program_options library])]
)

# --- PTHREAD ---------------------------------------------------
AC_CHECK_HEADERS([pthread.h], [], [AC_MSG_ERROR([Can't find pthread lib headers])])
AC_HAVE_LIBRARY(
	pthread,
	[], 
	[AC_MSG_ERROR([Can't find pthread library])]
)


AC_OUTPUT(Makefile src/Makefile)
