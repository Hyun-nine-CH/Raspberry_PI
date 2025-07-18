savedcmd_/home/rasp/folder/GPIO_F/gpioirq_module.mod := printf '%s\n'   gpioirq_module.o | awk '!x[$$0]++ { print("/home/rasp/folder/GPIO_F/"$$0) }' > /home/rasp/folder/GPIO_F/gpioirq_module.mod
