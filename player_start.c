#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include "player_ui.h"
#include "./encoding/gb2312_ucs2.h"
#include "./LRC/lyrics_process.h"

//此线程与子进程Mplayer通信 发送命令
void *pthread_send_player_cmd(void *arg)
{	
	MUI *mplayer =(MUI *)arg;
	mplayer->pause_flag = 0;//初始化暂停标志位
	char *buf[]={"get_time_pos\n","get_time_length\n","get_file_name\n","get_meta_artist\n","get_percent_pos\n"};
	int i=0;
	while(1)
	{
		for(i=0;i<5;i++)
		{
			if(mplayer->pause_flag == 0)//每发命令都会判断是否有暂停命令
			{
				write(mplayer->fd_pipe,buf[i],strlen(buf[i]));//通过命名管道给mplayer发命令
				usleep(200*1000);
			}
		}
		
	}
	return NULL;
}

//此线程接收Mplayer本来要输出到屏幕 而被重定向 写到无名管道中的信息
void *pthread_rcv_player_msg(void *arg)
{
	MUI *mplayer =(MUI *)arg;
	char src[256] = "";
	while(1)
	{
		char buf[1024]="";
		read(mplayer->fd[0],buf,sizeof(buf));//从无名管道读重定向后的mplayer的信息
		//printf("buf=%s\n",buf);
	
		if(strstr(buf,"ANS_FILENAME") != NULL)//歌名
		{
			sscanf(buf,"ANS_FILENAME='%[^']",mplayer->song);
			gtk_label_set_text(GTK_LABEL(mplayer->label_song_name), mplayer->song);
		}
		if(strstr(buf,"ANS_META_ARTIST") != NULL)//艺术家
		{
			sscanf(buf,"ANS_META_ARTIST='%[^']",src);
			gb2312_to_utf8((const unsigned char *)src, (unsigned char *)mplayer->artisit);
			gtk_label_set_text(GTK_LABEL(mplayer->label_singer), mplayer->artisit);
		}
		if(strstr(buf,"ANS_TIME_POSITION") != NULL)//当前时间
		{
			sscanf(buf,"ANS_TIME_POSITION=%s",mplayer->time_now);
			mplayer->time_1 = atoi(mplayer->time_now);
			sprintf(buf,"0%d:%d",mplayer->time_1/60, mplayer->time_1%60);
			gdk_threads_enter();
			gtk_label_set_text(GTK_LABEL(mplayer->cur_time), buf);
			gdk_threads_leave();
		}
		if(strstr(buf,"ANS_LENGTH") != NULL)//歌曲总时间
		{
			sscanf(buf,"ANS_LENGTH=%s",mplayer->time_all);
			mplayer->time_2 = atoi(mplayer->time_all);
			sprintf(buf,"0%d:%d",mplayer->time_2/60, mplayer->time_2%60);
			gtk_label_set_text(GTK_LABEL(mplayer->len_time), buf);			
		}
		float time_rate = (float)mplayer->time_1/mplayer->time_2;
		gdk_threads_enter();
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(mplayer->progressbar_rate), time_rate);
		gdk_threads_leave();
		
		if(strstr(buf,"ANS_PERCENT_POSITION") != NULL)//播放进度
		{
			sscanf(buf,"ANS_PERCENT_POSITION=%s",mplayer->time_rate);	
		}
	}
	return NULL;
}

//此线程用于显示歌词
void *pthread_show_lrc_msg(void *arg)
{
	while(1)
	{
		MUI *mplayer =(MUI *)arg;
		LRC_PTR lrc;
		LRC *head = NULL;
		char lyrics_pathname[128]="";
		char buf[128] = "";
		sscanf(mplayer->song, "%[^.]", buf);
		sprintf(lyrics_pathname, "../lyrics/%s.lrc", buf);
		
		head = dispose_lrc(lyrics_pathname, &lrc);
		
		if(head == NULL)//解析歌词失败
		{
			printf("No this lyrics\n");
		}
		else
		{
			int i;
			double time_now=atof(mplayer->time_now);
			//print_lrc_link(lrc);//打印解析出来的歌词与对应的时间
			for(i=0;i<lrc.lrc_arry_size;i++)//时间是毫秒ms
			{
				if((time_now*1000-lrc.lrc_arry[i]->time>-300) && (time_now*1000-lrc.lrc_arry[i]->time<300))
				{
					printf("time=%d, lrc==%s\n", lrc.lrc_arry[i]->time, lrc.lrc_arry[i]->lrc);
					char utf8[128] = "";
					gb2312_to_utf8((const unsigned char *)lrc.lrc_arry[i]->lrc, (unsigned char *)utf8);
					
					gdk_threads_enter();
					gtk_label_set_text((GtkLabel *)mplayer->label_lrc, utf8);
					gdk_threads_leave();
				}
			}
		}
		free_lrc_arry(&lrc);//释放歌词解析(dispose_lrc)后的资源
		usleep(100000);
	}
	return NULL;
}

int player_start(MUI *mplayer)
{
	int ret;
	pid_t pid;
	//char buf[100];
	printf("007\n");
	
	if(pipe(mplayer->fd)<0)//无名管道
		perror("pipe");
		
	ret = mkfifo("fifo_cmd", 0666);//命名管道
	if(ret < 0)
		perror("mkfifo");
	
	mplayer->fd_pipe = open("../execute/fifo_cmd", O_RDWR );
	printf("mplayer->fd_pipe open=%d\n",mplayer->fd_pipe);
	if(mplayer->fd_pipe < 0)
		perror("fd_pipe2");
	printf("00####)07\n");
	
	pid = vfork();//创建子进程
	if(pid < 0)
	{
		perror("vfork");
		exit(-1);
	}
	else if(pid == 0)//子进程
	{
		printf("this is son process\n");
		dup2(mplayer->fd[1], 1);//mplayer标准输出->无名管道写端
		execlp("mplayer",
			" mplayer", 
			"-slave", "-quiet","-idle",
			"-input", "file=../execute/fifo_cmd",
			"../song/Halo.mp3", NULL);
	}
	else
	{	
		printf("008\n");
	
		//memset(buf, 0, sizeof(buf));
		//fgets(buf, sizeof(buf), stdin);
		//write(fd_pipe, buf, strlen(buf));

		pthread_t pthread_send_cmd, pthread_rcv_msg;
		pthread_t pthread_show_lrc;
		
		pthread_create(&pthread_send_cmd,NULL,pthread_send_player_cmd,(void *)mplayer);//发送命令
	    pthread_create(&pthread_rcv_msg,NULL,pthread_rcv_player_msg,(void *)mplayer);//接收信息
		pthread_create(&pthread_show_lrc,NULL,pthread_show_lrc_msg,(void *)mplayer);//歌词显示

		pthread_detach(pthread_send_cmd);
		pthread_detach(pthread_rcv_msg);
		pthread_detach(pthread_show_lrc);
	}
	return 0;
}
