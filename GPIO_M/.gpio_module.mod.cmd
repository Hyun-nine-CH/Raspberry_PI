savedcmd_/home/rasp/folder/GPIO_M/gpio_module.mod := printf '%s\n'   gpio_module.o | awk '!x[$$0]++ { print("/home/rasp/folder/GPIO_M/"$$0) }' > /home/rasp/folder/GPIO_M/gpio_module.mod
