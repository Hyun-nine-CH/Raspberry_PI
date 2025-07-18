savedcmd_/home/rasp/folder/hello_module.mod := printf '%s\n'   hello_module.o | awk '!x[$$0]++ { print("/home/rasp/folder/"$$0) }' > /home/rasp/folder/hello_module.mod
