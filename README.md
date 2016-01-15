# hubic-backup

[hubiC](https://hubic.com/) backup command line tool for linux & OSX

## Features

* Exclude files using wildcards
* Aes 256bits encryption (openssl interop)
* Upload only modified files
* Logging support using excellent [spdlog](https://github.com/gabime/spdlog) library
* Multi threading support
* Incremental backup 

## Limitations

* [Large objects](http://docs.openstack.org/developer/swift/overview_large_objects.html) unsupported yet
* According to [this document](http://docs.openstack.org/developer/swift/api/object_api_v1_overview.html), more than 10000 files on a container unsupported yet.

## Dependencies

| Library | Ubuntu package  |
| ------- | --------------- |
| [libcurl](http://curl.haxx.se/libcurl/) | `libcurl4-openssl-dev` | 
| [boost-system](http://www.boost.org/doc/libs/1_55_0/libs/system/doc/index.html) |  `libboost-system-dev` | 
| [boost-filesystem](http://www.boost.org/doc/libs/1_57_0/libs/filesystem/doc/index.htm) | `libboost-filesystem-dev` |
| [boost-regex](http://www.boost.org/doc/libs/1_57_0/libs/regex/doc/html/index.html) | `libboost-regex-dev` |
| [boost-program-options](http://www.boost.org/doc/libs/1_57_0/doc/html/program_options.html) | `libboost-program-options-dev` |
| [jsoncpp](http://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html) | `libjsoncpp-dev` | 

## Setup

### Linux (Ubuntu)

*Triple-click on lines to select them.*

```bash
# Install build tools
sudo apt-get install git build-essential autoconf automake
# Clone this repository
git clone https://github.com/frachop/hubic-backup.git && cd hubic-backup/
# Install dependencies 
sudo apt-get install libboost-system-dev libcurl4-openssl-dev libboost-filesystem-dev libboost-regex-dev libboost-program-options-dev libjsoncpp-dev libssl-dev
# Launch automake
aclocal && automake && autoconf
# Build sources
./configure && make all
# Use the binary file
cp -v src/hubic-backup /usr/local/bin/
```
 
## Usage

### Description

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
  -d [ --del-non-existing ]          allow deleting non existing backup files
```

### Simple example

```
hubic-backup \
	--login <hubicLogin> \
	--pwd <hubicPassword> \
	--src /src/path/to/save/ \
	--dst hubic/backup/dir \
	--crypt-password @my8eau7ifulPa55w0rd!		
```

You can specify a particular container with `--container {containerName}` option.

You can specify a path to a file with excludes wildcards: `--excludes /path/of/exclude/file.txt`

Example of exclude file:

```
.*
tmp*
?tmpDir
```

To restore files, use any swift client or hubic browser interface. Then, if files are encrypted, use the command line: `openssl enc -aes-256-cbc -d -in <source path> -out <destination path> -k <password>` to decrypt it.
