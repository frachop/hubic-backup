/*************************************************************************/
/* hubic-backup - an fast and easy to use hubic backup CLI tool          */
/* Copyright (c) 2015 Franck Chopin.                                     */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

//[23:52:46.916I] [Auth] Credentials : token = '3e88142ea8c44415a366d966ad8a5d25', endpoint = 'https://lb1040.hubic.ovh.net/v1/AUTH_47e7e0fe42913821a6365ad2e220bcc5', expire = '2015-03-20T23:19:32+01:00'

#include "options.h"
#include "common.h"

//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <fstream>

namespace po = boost::program_options;

//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////

COptions::COptions()
:	_argc(0)
,	_argv(nullptr)
,	_hubicLogin()
,	_hubicPassword()
,	_srcFolder()
,	_excludes()
,	_dstContainer()
,	_dstFolder()
,	_cryptoPassword()
,	_authToken()
,	_authEndpoint()
,	_curlVerbose(false)
{
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EOptionFlag {
		Help
	,	Version
	,	logLevel
	
	,	hubicLogin
	,	hubicPwd
	
	,	srcFolder
	,	excludes
	,	dstContainer
	,	dstFolder
	,	cryptPassword
	
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EOptionGroup {
		general
	,	auth
	,	source
	,	destination
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////


static const std::map<EOptionGroup, std::string> _g {
		{ EOptionGroup::general    , "general" }
	,	{ EOptionGroup::auth       , "auth" }
	,	{ EOptionGroup::source     , "source" }
	,	{ EOptionGroup::destination, "destination" }
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SOption
{
public:
	const EOptionGroup _group;
	const char       * _key;
	const std::string  _description;
	const char       * _shortKey;
	SOption(EOptionGroup g, const char * k, const std::string& d, const char * shortKey= nullptr)
	:	_group(g)
	,	_key(k)
	,	_description(d)
	,	_shortKey(shortKey)
	{}
	
	std::string name() const {
		return (_shortKey) ? _key + std::string(",") + _shortKey : std::string(_key);
	}
};

//- LOG LEVEL /////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string getSeverityList() {
	using namespace spdlog;
	std::string result;
	for (auto i=0; i<level::level_enum_count; ++i) {
		result += fmt::format("'{}'", level::level_names[i]);
		result += (i == level::level_enum_count - 2) ? " or " : (i == level::level_enum_count - 1) ? "" : ", ";
	}
	
	return result;
}

void setLogSeverity(const std::string & s) {
	using namespace spdlog;
	std::string result;
	for (auto i=0; i<level::level_enum_count; ++i)
		if (s == level::level_names[i]) {
			LOGGER->set_level(static_cast<level::level_enum>(i));
			return;
		}
			
	throw std::logic_error("invalid log level : '" + s + "'");
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static const std::map<EOptionFlag, SOption> _o {
		{EOptionFlag::Help         , { EOptionGroup::general    , "help"          , "this message"         , "h" }}
	,	{EOptionFlag::Version      , { EOptionGroup::general    , "version"       , "display version infos", "v" }}
	,	{EOptionFlag::logLevel     , { EOptionGroup::general    , "loglevel"      , "select the log level. (" + getSeverityList() + ")"  }}
	
	,	{EOptionFlag::hubicLogin   , { EOptionGroup::auth       , "login"         , "hubic login"    , "l"}}
	,	{EOptionFlag::hubicPwd     , { EOptionGroup::auth       , "pwd"           , "hubic password" , "p"}}
	
	,	{EOptionFlag::srcFolder    , { EOptionGroup::source     , "src"           , "source folder", "i" }}
	,	{EOptionFlag::excludes     , { EOptionGroup::source     , "excludes"      , "optional exclude file list path", "x" }}
	,	{EOptionFlag::dstContainer , { EOptionGroup::destination, "container"     , "destination hubic container", "c" }}
	,	{EOptionFlag::dstFolder    , { EOptionGroup::destination, "dst"           , "destination folder", "o" }}
	,	{EOptionFlag::cryptPassword, { EOptionGroup::destination, "crypt-password", "optional crypto password", "k" }}
	
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class COptionsPriv
:	public po::variables_map
,	public COptions
{
public:
	COptionsPriv() {}
	bool parse(int argc, char** argv);
	bool exists( EOptionFlag f ) const { return count( _o.at(f)._key ) > 0; }
	po::variable_value at(EOptionFlag f) const { return po::variables_map::at( _o.at(f)._key ); }
	
};

static po::value_semantic* getDefaultValue(EOptionFlag f)
{
	static COptionsPriv _p;
	switch (f) {
		case EOptionFlag::Help         : break;
		case EOptionFlag::Version      : break;
		case EOptionFlag::logLevel     : return po::value<std::string>()->default_value(spdlog::level::to_str( LOGGER->level() ));

		case EOptionFlag::hubicLogin   : return po::value<std::string>();
		case EOptionFlag::hubicPwd     : return po::value<std::string>();
		case EOptionFlag::srcFolder    : return po::value<std::string>();
		case EOptionFlag::excludes     : return po::value<std::string>();
		case EOptionFlag::dstContainer : return po::value<std::string>()->default_value("default");
		case EOptionFlag::dstFolder    : return po::value<std::string>();

		case EOptionFlag::cryptPassword: return po::value<std::string>();
	};
	return new po::untyped_value(true);
}

static boost::shared_ptr<po::option_description> getDesc(EOptionFlag f, const SOption& o)
{
	return boost::shared_ptr<po::option_description>(
		new po::option_description(o.name().c_str(), getDefaultValue(f), o._description.c_str())
	);
}

//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void loadExcludes(std::set<std::string> & x, const boost::filesystem::path & ePath)
{
	using namespace boost::filesystem;
	if (!exists(ePath))
		throw std::logic_error(fmt::format("exclude file list path doesn't exists '{}'", ePath.string()));

	std::ifstream f( ePath.c_str() );
	if (!f.is_open())
		throw std::logic_error(fmt::format("error while opening exclude list file '{}'", ePath.string()));
	
	std::string line;
	while(getline(f, line)) {
		line = boost::trim_copy(line);
		if (line.empty()) continue;
		x.insert(line);
	}
	if (f.bad())
		throw std::logic_error(fmt::format("error while reading exclude list file '{}'", ePath.string()));
}


//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CHECK_MANDATORY_ARG( a ) if (!exists( (a) )) throw std::logic_error(fmt::format("miss mandatory arg --{}", _o.at( (a) )._key));

static std::string trimRightSlash( const std::string & src)
{
	if (src.empty())
		return "";
	
	std::string res(src);
	while (res[res.length()-1] == '/') {
		res = res.substr(0, res.length() - 1);
		if (res.empty())
			break;
	}
	return res;
}

bool COptionsPriv::parse(int ac, char** av)
{
	_argc= ac;
	_argv= av;
	
	
	po::options_description visible;
	for ( auto g : _g )
	{
		po::options_description group(g.second);
		for ( auto o : _o ) {
			if (o.second._group != g.first) continue;
			group.add( getDesc(o.first, o.second) );
		}
		visible.add(group);
	}
	
	po::options_description hiddens;
	hiddens.add( boost::shared_ptr<po::option_description>(
		new po::option_description("auth-token", po::value<std::string>(), "credential token")
	));
	hiddens.add( boost::shared_ptr<po::option_description>(
		new po::option_description("auth-endpoint", po::value<std::string>(), "endpoint")
	));
	hiddens.add( boost::shared_ptr<po::option_description>(
		new po::option_description("curl-verbose", po::value<std::string>(), "curl verbose on/off")
	));
	
	po::options_description all;
	all.add(visible);
	all.add(hiddens);

	try
	{
		po::parsed_options parsed =
			po::command_line_parser(ac, av)
			.	options(all)
		//	.	allow_unregistered()
			.	run()
		;
		
		po::store(parsed, *this);
		po::notify(*this);
		
		if (exists(EOptionFlag::Help)) {
			std::cout << "Usage: " << boost::filesystem::path(av[0]).filename().string() << " [OPTIONS]" << std::endl;
			std::cout << visible << std::endl;
			return false;
		}
		
		if (exists(EOptionFlag::Version)) {
			std::cout
				<< boost::filesystem::path(av[0]).filename().string() << " v" << HUBACK_VERSION
				//<< std::endl
				//<< "   Git commit Id : " << getGitCommitIdString()
				<< std::endl;
			return false;
		}

		if (exists(EOptionFlag::logLevel))
			setLogSeverity(at(EOptionFlag::logLevel).as<std::string>());

		if (!exists(EOptionFlag::hubicLogin)) {
		
				if (count("auth-token") && count("auth-endpoint")) {
				_authToken   = po::variables_map::at( "auth-token" ).as<std::string>();
				_authEndpoint= po::variables_map::at( "auth-endpoint" ).as<std::string>();
			}
		
		} else {

			CHECK_MANDATORY_ARG(EOptionFlag::hubicLogin);
			_hubicLogin = at(EOptionFlag::hubicLogin).as<std::string>();
			
			CHECK_MANDATORY_ARG(EOptionFlag::hubicPwd);
			_hubicPassword = at(EOptionFlag::hubicPwd).as<std::string>();
		}

		CHECK_MANDATORY_ARG(EOptionFlag::srcFolder);
		_srcFolder = trimRightSlash( at(EOptionFlag::srcFolder).as<std::string>());

		if (!boost::filesystem::exists(_srcFolder))
			 throw std::logic_error(fmt::format("src folder '{}' doesn't exists", _srcFolder.string()));
		
		if (!boost::filesystem::is_directory(_srcFolder))
			 throw std::logic_error(fmt::format("src folder '{}' is not a folder", _srcFolder.string()));
		
		if (exists(EOptionFlag::excludes))
			loadExcludes(_excludes, at(EOptionFlag::excludes).as<std::string>());
		
		if (exists( EOptionFlag::dstContainer))
			_dstContainer= at(EOptionFlag::dstContainer).as<std::string>();
			
		CHECK_MANDATORY_ARG(EOptionFlag::dstFolder);
		_dstFolder = at(EOptionFlag::dstFolder).as<std::string>();

		if (exists( EOptionFlag::cryptPassword)) 
			_cryptoPassword = at(EOptionFlag::cryptPassword).as<std::string>();

		if (count("curl-verbose")) {
			_curlVerbose = (po::variables_map::at( "curl-verbose" ).as<std::string>() == "on");
		}

	}
	catch (const std::exception & e)
	{
		LOGC("{}. {}", e.what(), "Application will now exit.");
		exit( EXIT_FAILURE );
	}
	catch (...)
	{
		LOGC("{}", "Unhandled Exception reached the top of main while parsing arguments. Application will now exit.");
		exit( EXIT_FAILURE );
	}
	
#define S_LIB "{:15s}:"
	LOGI("program started");
	LOGI("version {}", HUBACK_VERSION);
	//LOGI("{} v{}", boost::filesystem::path(av[0]).filename().string(), getVersionString());
	//LOGI("Git commit Id : %s", getGitCommitIdString().c_str());
	LOGI("with settings :");
	LOGI(S_LIB " {}", "Hubic login", _hubicLogin);
	LOGI(S_LIB " {}", "Sources folder", _srcFolder);
	for (const auto & s : _excludes)
		LOGI(S_LIB " {}", "excludes", s);
	LOGI(S_LIB " {}", "Container", _dstContainer);
	LOGI(S_LIB " {}", "Destination", _dstFolder);
	LOGI(S_LIB " {}", "Crypted ?", crypted() ? "yes" : "no");

	return true;
}

//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////

static COptions * s_pArgs(00);
const COptions * COptions::get(int argc, char** argv)
{
	static COptionsPriv s_args;
	assert(s_pArgs == 00);
	
	s_pArgs = (s_args.parse( argc, argv) ? (&s_args) : 00);
	return s_pArgs;
	
}
const COptions * COptions::get()
{
	return s_pArgs;
}


