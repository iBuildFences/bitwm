wm : main.c bin_tree.c bin_tree.h
	gcc -std=c99 -g3 main.c bin_tree.c bin_tree.h -o wm -lxcb -lxcb-keysyms

tags : ./*
	   ctags --c-kinds=+xp -R ./* /usr/include/xcb/* /usr/include/stdio.h /usr/include/stdlib.h
