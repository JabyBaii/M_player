#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include "sungtk_interface/sungtk_interface.h"
#include "player_song_list.h"
#include "./encoding/gb2312_ucs2.h" 
#include"player_ui.h"

MUI *tep=NULL;

//上一曲回调
void callback_button_back(GtkWidget *button_pause, gpointer data)
{	
	MUI *mplayer =(MUI *)data;
	char buf[128]="";
	if(mplayer->num_song == FIRST_SONG)
		mplayer->num_song = LAST_SONG;
	else
		mplayer->num_song--;
	sprintf(buf,"loadfile ../song/%s\n",mplayer->song_list[mplayer->num_song]);
	printf("buf=%s\n",buf);
	write(mplayer->fd_pipe,buf,strlen(buf));
}

//暂停&播放回调
void callback_button_pause(GtkWidget *button_pause, gpointer data)
{	
	MUI *mplayer =(MUI *)data;
	if(mplayer->pause_flag == 0)
	{
		sungtk_button_set_image(button_pause, 
		"../picture/button_style/pause.png", 80, 80);//重新给按钮设置照片
		mplayer->pause_flag = 1;
	}
	else if(mplayer->pause_flag == 1)
	{
		sungtk_button_set_image(button_pause, 
		"../picture/button_style/play.png", 80, 80);//
		mplayer->pause_flag = 0;
	}
	write(mplayer->fd_pipe, "pause\n", strlen("pause\n"));
}

//下一曲回调
void callback_button_next(GtkWidget *button_pause, gpointer data)
{
	MUI *mplayer =(MUI *)data;
	char buf[128]="";
	if(mplayer->num_song == LAST_SONG)
		mplayer->num_song = FIRST_SONG;
	else
		mplayer->num_song++;
	sprintf(buf,"loadfile ../song/%s\n",mplayer->song_list[mplayer->num_song]);
	printf("buf=%s\n",buf);
	write(mplayer->fd_pipe,buf,strlen(buf));
}


/*
static gboolean button_pressed( GtkWidget *eventbox, GdkEventButton *event, GtkLabel *label )
{
	//选中的歌曲 高亮显示歌名
	if (event->type == GDK_2BUTTON_PRESS)//双击
	{
		sungtk_clist_set_text_size((SunGtkCList *)mplayer.label[i], 20);
		
		const gchar *text = gtk_label_get_text(label);
		if( text[0] == 'D' ){
			gtk_label_set_text(label, "I Was Double-Clicked!");
		}else{
			gtk_label_set_text(label, "Double-Click Me Again!");
		}
		
	}
	
	return FALSE;
}
*/
static gboolean callback_list_release(GtkWidget *widget, GdkEventButton *event, gpointer data)  
{  
    int row = (int)data; 
	char buf[100]="";
    printf("row===%d\n", row);  
      
    const char *text = sungtk_clist_get_row_data(tep->list, row);  
    printf("text==%s==\n", text);  
	sprintf(buf,"loadfile ../song/%s\n",text);
	write(tep->fd_pipe,buf,strlen(buf));
    return TRUE;  
}  

//播放进度回调
gboolean play_progress_call( GtkWidget *widget,GdkEventButton *event,gpointer arg )
{
	MUI *mplayer =(MUI *)arg;
	
	printf("x=%lf\n",event->x);
	printf("y=%lf\n",event->y);
	char buf[100]="";
	double x = event->x;
	double progress = x/400;//播放进度
	
	gtk_progress_bar_set_fraction((GtkProgressBar *)mplayer->progressbar_rate, progress);
	sprintf(buf,"seek %d\n",(int)(progress*100));
	write(mplayer->fd_pipe,buf,strlen(buf));
	
	
	return TRUE;
}


//音量回调
gboolean volume_call( GtkWidget *widget,GdkEventButton *event,gpointer arg )
{
	MUI *mplayer =(MUI *)arg;
	
	if( event->y <= 75 ) 
	{
		printf("x=%lf\n",event->x);
		char buf[100]="";
		double y = 75 - event->y;
		printf("y=%lf\n",y);
		double volume = y/75;//音量百分比
		
		gdk_threads_enter();
		gtk_progress_bar_set_fraction((GtkProgressBar *)mplayer->progressbar_volume, volume);
		gdk_threads_leave();
		sprintf(buf,"volume %d 1\n",(int) (volume*100));
		write(mplayer->fd_pipe,buf,strlen(buf));
	}
	else
	{
		static int flag=0;
		if(flag == 0)
		{
			gdk_threads_enter();
			sungtk_image_load_picture(mplayer->image_volume, "../picture/muteoff.jpg", 70, 60);//更改图片
			gdk_threads_leave();
			write(mplayer->fd_pipe, "mute 1\n", strlen("mute 1\n"));
			flag++;
		}
		else if(flag == 1)
		{
			gdk_threads_enter();
			sungtk_image_load_picture(mplayer->image_volume, "../picture/mute.jpg", 70, 60);//更改图片
			gdk_threads_leave();
			write(mplayer->fd_pipe, "mute 0\n", strlen("mute 0\n"));
			flag--;
		}
		
	}
	
	return TRUE;
}

int player_ui(MUI *mplayer)
{
	tep = mplayer;
	mplayer->num_song = 1;
	mplayer->gxml = glade_xml_new("../glade/mplayer.glade",NULL,NULL);	//第一步，打开glade文件，返回类型GladeXML

	mplayer->window = glade_xml_get_widget(mplayer->gxml,"window_back");//第二步，通过该接口，获取创建的控件
	sungtk_background_set_picture(mplayer->window, "../picture/background/11.jpg", 800, 480);//设置窗口背景图片
	
	
	//fixed = glade_xml_get_widget(mplayer_ui.gxml,"fixed_back");//获取到固定布局，通过代码可以继续添加控件
	//歌曲列表  添加透明列表
	mplayer->fixed_list = glade_xml_get_widget(mplayer->gxml,"fixed_list");//获取歌曲列表控件
	mplayer->list = sungtk_clist_new();  
    sungtk_clist_set_row_height(mplayer->list, 30);  
    sungtk_clist_set_col_width(mplayer->list, 200);  
    sungtk_clist_set_text_size(mplayer->list, 13);  
	sungtk_clist_set_select_row_signal(mplayer->list, "button-release-event", callback_list_release);  
	
	
	//mplayer->vbox = gtk_vbox_new(TRUE, 10);       // 创建纵向盒状容器
	gtk_fixed_put(GTK_FIXED(mplayer->fixed_list), mplayer->list->fixed, 0, 0); // 将列表的固定布局放入歌曲列表布局
	int i = 0;//歌名数量					
	while(mplayer->song_list[i] )
	{
		//printf("0000\n");
		sungtk_clist_append(mplayer->list,mplayer->song_list[i] ); 
		// mplayer->eventbox[i] = gtk_event_box_new();				// 事件盒子的创建
		// gtk_event_box_set_visible_window((GtkEventBox *)mplayer->eventbox[i], FALSE);//设置事件盒子不可见
		// gtk_container_add(GTK_CONTAINER(mplayer->vbox), mplayer->eventbox[i]);	// 事件盒子放入窗口
		// mplayer->label[i] = gtk_label_new(mplayer->song_list[i]);	// label  
		
		//sungtk_clist_set_text_size((SunGtkCList *)mplayer->song_list[i], 20);
		
		//gtk_container_add( GTK_CONTAINER(mplayer->eventbox[i]), mplayer->label[i]);	// label放入事件盒子里
		//printf("Name%d:%s\n", i, mplayer->song_list[i]);
		//g_signal_connect(mplayer->eventbox[i], "button_press_event", G_CALLBACK(button_pressed), (gpointer)mplayer->label[i]);
		i++;					
	}
	
	//上一曲
	mplayer->button_back = glade_xml_get_widget(mplayer->gxml,"button_back");//获取按钮控件
	gtk_button_set_label(GTK_BUTTON(mplayer->button_back), "");
	sungtk_button_inset_image(mplayer->button_back, 
				"../picture/button_style/back.png", 80, 80);//按钮第一次插入一张图片
	gtk_button_set_relief((GtkButton *)mplayer->button_back, GTK_RELIEF_NONE);//透明背景
	g_signal_connect(mplayer->button_back, 
				"pressed", G_CALLBACK(callback_button_back), mplayer);
	
	//暂停&播放
	mplayer->button_pause = glade_xml_get_widget(mplayer->gxml,"button_pause");//获取按钮控件
	gtk_button_set_label(GTK_BUTTON(mplayer->button_pause), "");
	sungtk_button_inset_image(mplayer->button_pause, 
				"../picture/button_style/play.png", 80, 80);//按钮第一次插入一张图片
	gtk_button_set_relief((GtkButton *)mplayer->button_pause, GTK_RELIEF_NONE);//透明背景
	g_signal_connect(mplayer->button_pause, 
				"pressed", G_CALLBACK(callback_button_pause), mplayer);
	printf("001\n");
	
	//下一曲
	mplayer->button_next = glade_xml_get_widget(mplayer->gxml,"button_next");//获取按钮控件
	gtk_button_set_label(GTK_BUTTON(mplayer->button_next), "");
	sungtk_button_inset_image(mplayer->button_next, 
				"../picture/button_style/front.png", 80, 80);//按钮第一次插入一张图片
	gtk_button_set_relief((GtkButton *)mplayer->button_next, GTK_RELIEF_NONE);//透明背景
	g_signal_connect(mplayer->button_next, 
				"pressed", G_CALLBACK(callback_button_next), mplayer);
	printf("002\n");
	
	//音量图片
	mplayer->image_volume = glade_xml_get_widget(mplayer->gxml,"image_volume");//获取volume图片控件
	mplayer->eventbox_volume = glade_xml_get_widget(mplayer->gxml,"eventbox_volume");
	
	printf("mplayer->image_volume=%p\n",mplayer->image_volume);
	sungtk_image_load_picture(mplayer->image_volume, "../picture/mute.jpg", 70, 60);//更改图片
	
	//音量大小
	mplayer->progressbar_volume = glade_xml_get_widget(mplayer->gxml,"progressbar_volume");//获取volume图片控件
	gtk_progress_bar_set_orientation((GtkProgressBar *)mplayer->progressbar_volume,GTK_PROGRESS_BOTTOM_TO_TOP);//移动方向 从下至上
	g_signal_connect(mplayer->eventbox_volume, "button-press-event", G_CALLBACK(volume_call), mplayer );
	
	//播放进度
	mplayer->progressbar_rate = glade_xml_get_widget(mplayer->gxml,"progressbar_rate");//获取按钮控件
	gtk_progress_bar_set_orientation((GtkProgressBar *)mplayer->progressbar_rate,GTK_PROGRESS_LEFT_TO_RIGHT);//移动方向 从左至右
	g_signal_connect(mplayer->progressbar_rate, "button-press-event", G_CALLBACK(play_progress_call), mplayer);
	
	//当前播放时间
	mplayer->cur_time = glade_xml_get_widget(mplayer->gxml,"label_cur_time");
	gtk_label_set_text(GTK_LABEL(mplayer->cur_time), "00:00");
	
	//歌曲总时间
	mplayer->len_time = glade_xml_get_widget(mplayer->gxml,"label_len_time");
	gtk_label_set_text(GTK_LABEL(mplayer->len_time), "00:00");
	
	//歌词
	mplayer->label_lrc = glade_xml_get_widget(mplayer->gxml,"label_lrc");//获取label控件
	gtk_label_set_text(GTK_LABEL(mplayer->label_lrc), "");
	
	//歌名
	mplayer->label_song_name = glade_xml_get_widget(mplayer->gxml,"label_song_name");//获取label控件
	gtk_label_set_text(GTK_LABEL(mplayer->label_song_name), "");
	
	//艺术家名
	mplayer->label_singer = glade_xml_get_widget(mplayer->gxml,"label_singer");//获取label控件
	gtk_label_set_text(GTK_LABEL(mplayer->label_singer), "");
	
	//艺术家图片
	mplayer->image_artist = glade_xml_get_widget(mplayer->gxml,"image_artist");//获取artist图片控件
	sungtk_image_load_picture(mplayer->image_artist, "../picture/music.png", 150, 150);//更改图片
	
	
	
	//回调函数
	
	//g_signal_connect(buttons,"pressed",G_CALLBACK(callback_sure),entry);
	//g_signal_connect(buttond,"pressed",G_CALLBACK(callback_del),entry);
	//g_signal_connect(button1,"pressed",G_CALLBACK(callback_1),entry);
	
	//设置文字
	//GtkWidget *label = gtk_label_new("Test Font");	// 标签
	//set_widget_font_size(label, 50, FALSE);			// 设置字体大小
	//gtk_container_add(GTK_CONTAINER(mplayer_ui.window), label);		// 把盒子放在窗口里
	
	// 设置窗口背景图片
	//sungtk_background_set_picture(mplayer_ui.window, "../picture/background/11.jpg", 1000, 600);	
	
	gtk_widget_show_all(mplayer->window); // 显示所有部件
	printf("004\n");
	return 0;
}
