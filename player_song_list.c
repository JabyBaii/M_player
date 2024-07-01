#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include"player_song_list.h"



int read_song_dir(MUI *player)
{
	player->song_list[100]="";
	DIR *dir;
	struct dirent *dirp;
	int i = 0;

	if((dir=opendir("../song"))==NULL)
	{
		printf("Open dir song fail\n");
		exit(1);
	}

	while((dirp=readdir(dir))!=NULL)
	{
		if(strstr(dirp->d_name, ".mp3"))//处理歌曲名字
		{
			player->song_list[i]=(char*)malloc(sizeof(dirp->d_name)+1);
			strcpy(player->song_list[i],dirp->d_name);	
			i++;
		}		
	}
	closedir(dir);

	return (0);
}
