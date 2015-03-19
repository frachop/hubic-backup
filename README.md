# hubic-backup

[Hubic](http://hubic.com) backup command line tool for linux & OSX

For the moment only in pre-alpha.

##Features:

* Exclude files using wildcards
* encrypt (openssl interop)
* upload only modified files
* logging support using excellent [spdlog](https://github.com/gabime/spdlog) library

##dependencies

* [boost-system](http://www.boost.org/doc/libs/1_55_0/libs/system/doc/index.html)
* [boost-filesystem](http://www.boost.org/doc/libs/1_57_0/libs/filesystem/doc/index.htm)
* [boost-regex](http://www.boost.org/doc/libs/1_57_0/libs/regex/doc/html/index.html)
* [jsoncpp](http://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html)

## usage

simple example :

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
