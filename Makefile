src += main.c
src += player_ui.c
src += sungtk_interface/sungtk_interface.c
src += player_song_list.c
src += player_start.c
src += ./LRC/lyrics_process.c
src += ./encoding/gb2312_ucs2.c
target = ../execute/play


cc = gcc 
obj = $(src:%.c=%.o)

#使用glade需要加上 参数libglade-2.0
cflags = `pkg-config --cflags --libs gtk+-2.0` `pkg-config --cflags --libs libglade-2.0` -lpthread 
option = -Wall
$(target):$(obj)
	$(cc) $^ -o $@ $(cflags) $(option)
%.o:%.c
	$(cc) -c $< -o $@  $(cflags) $(option)
	
clean:
	rm *.o $(targer) -rf

	
