# hubic-backup

[Hubic](http://hubic.com) backup command line tool for linux & OSX

##Features:

* Exclude files using wildcards
* aes 256bits encryption (openssl interop)
* upload only modified files
* logging support using excellent [spdlog](https://github.com/gabime/spdlog) library
* multi threading support

##Limitations

* [Large objects](http://docs.openstack.org/developer/swift/overview_large_objects.html) unsupported yet

##Dependencies

|Library|Ubuntu packet|
|-------|--------------|
| [libcurl](http://curl.haxx.se/libcurl/) | ```libcurl4-openssl-dev``` | 
| [boost-system](http://www.boost.org/doc/libs/1_55_0/libs/system/doc/index.html) |  ```libboost-system-dev``` | 
| [boost-filesystem](http://www.boost.org/doc/libs/1_57_0/libs/filesystem/doc/index.htm) | ```libboost-filesystem-dev``` | 
| [boost-regex](http://www.boost.org/doc/libs/1_57_0/libs/regex/doc/html/index.html) | ```libboost-regex-dev``` | 
| [boost-program-options](http://www.boost.org/doc/libs/1_57_0/doc/html/program_options.html) | ```libboost-program-options-dev``` | 
| [jsoncpp](http://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html) | ```libjsoncpp-dev``` | 

##Setup
###Linux (ubuntu)

* clone sources files ```git clone https://github.com/frachop/hubic-backup.git```
* install dependencies ```sudo apt-get install libboost-system-dev libcurl4-openssl-dev libboost-filesystem-dev libboost-regex-dev libboost-program-options-dev libjsoncpp-dev```
* launch automake ```aclocal && automake && autoconf```
* build sources ```./configure && make all```
* use the binary file which path is ```src/hubic-backup```
 
##Usage

###Description :

```
Usage: hubic-backup [OPTIONS]

general:
  -h [ --help ]                      this message
  -v [ --version ]                   display version infos
  --loglevel arg (=trace)            select the log level. ('trace', 'debug', 
                                     'info', 'notice', 'warning', 'error', 
                                     'critical', 'alert' or 'emerg')

auth:
  -l [ --login ] arg                 hubic login
  -p [ --pwd ] arg                   hubic password

source:
  -i [ --src ] arg                   source folder
  -x [ --excludes ] arg              optional exclude file list path

destination:
  -c [ --container ] arg (=default)  destination hubic container
  -o [ --dst ] arg                   destination folder
  -k [ --crypt-password ] arg        optional crypto password
```
###Simple example :

```
hubic-backup \
	--login <hubicLogin> \
	--pwd <hubicPassword> \
	--src /src/path/to/save/ \
	--dst hubic/backup/dir \
	--crypt-password @my8eau7ifulPa55w0rd!		
```

You can specify a particular container with ```--container {containerName}``` option.

You can specify a path to a file with excludes wildcards : ```--excludes /path/of/exclude/file.txt``` 

Example of exclude file:

```
.*
tmp*
?tmpDir
```

To restore files, use any swift client or hubic browser interface. Then, if files are encrypted, use the command line : ```openssl enc -aes-256-cbc -d -in <source path> -out <destination path> -k <password>``` to decrypt it.

