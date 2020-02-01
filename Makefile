wm : main.c bin_tree.c bin_tree.h tags.c tags.h util.c util.h bindings.c bindings.h
	gcc -std=c99 -g3 -fdiagnostics-color=always main.c bin_tree.c tags.c util.c bindings.c -o wm -lxcb -lxcb-keysyms

exec : wm
	sudo cp wm /usr/bin/bitwm
	sudo cp bitwm.desktop /usr/share/xsessions

tags : ./*
	   ctags --c-kinds=+xp -R ./* #/usr/include/xcb/* /usr/include/stdio.h /usr/include/stdlib.h
