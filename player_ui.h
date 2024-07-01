#ifndef __PLAYER_UI_H_
#define __PLAYER_UI_H_
#include <glade/glade.h>
#include "sungtk_interface/sungtk_interface.h"



typedef struct mplayer_ui
{
	GladeXML *gxml; 
	GtkWidget *window;
	//GtkWidget *fixed;
	GtkWidget *fixed_list;
	GtkWidget *vbox;
	GtkWidget *eventbox[20];
	GtkWidget *label[20];
	GtkWidget *button_back, *button_pause, *button_next;
	GtkWidget *progressbar_rate;//播放进度条
	GtkWidget *cur_time;
	GtkWidget *len_time;
	GtkWidget *label_song_name, *label_singer;
	GtkWidget *image_volume, *image_artist;
	GtkWidget *volume;
	GtkWidget *label_lrc;
	
	int fd[2];//无名
	int fd_pipe;//命名
	GtkWidget *eventbox_volume;
	GtkWidget *progressbar_volume;
	
	char artisit[128];
	char song[128];
	char time_now[128];
	char time_all[128];
	char time_rate[128];
    int pause_flag;//暂停标志位
	
	int time_1;
	int time_2;
	
	char *song_list[100];//获取歌曲名字
	int num_song;
	
	SunGtkCList *list;
	
}MUI;

//窗口界面函数
extern int player_ui(MUI *mplayer);

#endif


	