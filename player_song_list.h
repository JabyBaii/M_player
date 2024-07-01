#ifndef __PLAYER_SONG_LIST_H_
#define __PLAYER_SONG_LIST_H_
#include"player_ui.h"

#define FIRST_SONG 0
#define LAST_SONG 17//歌曲数目

//读取歌曲列表
extern int read_song_dir(MUI *player);

#endif