//
// Created by Wanning Pan on 2019/3/5.
//

#ifndef LAB5_FILE_SYSTEM_H
#define LAB5_FILE_SYSTEM_H

#include "structure.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <ctime>
extern fstream Disk;

class file_system {
public:
    bool login(const char* user_name,const char* pwd);
    void run(bool tag);
    file_system();
    ~file_system();
private:
    /*整体文件系统信息*/
    super_block sp;

    /*工作目录信息*/
    inode *i_cur;
    directory *dir_cur;
    //当前用户信息
    int cur_gid;

    /*根目录常驻内存*/
    inode i_root;
    directory root;
    directory buffer;

    /*初始化*/
    //格式化
    bool format_fs();


    /*路径解析*/
    //buffer为本身目录项
    int namei(const char* dir_name);
    //buffer为父母目录项
    int namei(const char* dir_name,int &par_num);
    //获取父母&本身所在inode
    int namei(const char* dir_name,int &par_num,pair<int,int>&loc);


    /*读写文件*/
    bool open_dir(int inode_num,inode *cur_node,directory *cur_dir);
    bool open_dir(inode *cur_node,directory *cur_dir);
    //将当前工作目录数据及inode相关写回磁盘
    bool close_dir(const inode *i_w,directory * dir_w);

    /*基本命令实现*/
    //ls
    bool list_dir();
    //cd
    bool chdir(const char *dir_name);
    //mkdir
    bool make_dir(const char *dir_name);
    //touch
    bool touch(const char * file_name);
    //rm
    bool remove_dir(const char* file_name);
    //delete inode
    bool del_inode(inode&);
    //mv
    bool move_file(const char*from_name,const char*to_name);
    //cat
    bool cat(const char* file_name);
    //echo
    bool echo(const char* input,const char*file_name);

};

#endif //LAB5_FILE_SYSTEM_H
