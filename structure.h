//
// Created by Wanning Pan on 2019/3/4.
//

#ifndef LAB5_FS_H
#define LAB5_FS_H

#include <cstring>
#include <vector>
#include <string>
#include <cstdio>
#include <fstream>
using namespace std;

#define DISK "disk.img"
#define USER_INF "conf.txt"

#define NAME_LEN 62
#define N_BLOCKS 15
#define INODE_SIZE sizeof(inode)
#define BLOCK_SIZE (1024*sizeof(char))
#define SP_SIZE sizeof(super_block)

#define ROOT_NODE 99
#define DIR_LEN 15
#define FREE_STACK_NUM 100
#define INODE_NUM 4096
#define BLOCK_NUM 61440
#define MAX_DIR_NUM 15
#define USER_NUM 4

#define FILE_TYPE 0x0f
#define DIR_TYPE 0xf0

#define SUPER_BEGIN 0
#define INODE_BEGIN sizeof(super_block)
#define BLOCK_BEGIN (INODE_BEGIN + sizeof(inode) * INODE_NUM)

extern fstream Disk;

struct super_block{

    bool inode_bitmap[INODE_NUM];
    bool block_bitmap[BLOCK_NUM];
    char user_map[USER_NUM][NAME_LEN];
    char pwd_map[USER_NUM][NAME_LEN];
    int user_gid[USER_NUM];
    /*空闲管理*/
    //直接管理的空闲块数目
    int s_nfree;
    //直接管理的inode数目
    int s_ninode;
    //空闲块
    int s_free[FREE_STACK_NUM];
    //空闲inode
    int s_inode[FREE_STACK_NUM];

    //inode数量
    int inodes_count;
    //数据块数量
    int blocks_count;

    //空闲数据块数量
    int free_blocks_count;
    //空闲INODE数量
    int free_inodes_count;
    //第一个空闲数据块
    int first_free_block;
    //第一个剩余INODE
    int first_free_inode;
    //数据块大小,bytes
    int block_size;
    //INODE大小
    int inode_size;

    //分配新数据块
    int get_new_inode();
    int get_new_block();
    //回收数据块
    bool delete_inode(int inode_num);
    bool delete_block(int blk_num);

    //将此块数据写入
    bool write();
    //从磁盘读出此块数据
    bool read();

    super_block();
};

/*inode,96Bytes*/
struct inode{
    //读写、执行模式
    short i_mode;
    //文件类型
    short i_type;
    //用户ID16位
    short i_uid;
    //用户组ID低16位
    short i_gid;
    //文件大小
    unsigned int i_size;
    //指向数据块的指针,0-11为直接寻址(1-12BLOCKSIZE)，[12,13,14]为间接寻址(12*(BLKSIZE/4)*BLKSIZE*...)
    int i_blocks[N_BLOCKS];
    //创建时间
    time_t i_atime;
    //修改时间
    time_t i_mtime;
    //数据块数量
    unsigned int i_blocks_count;
    //inode节点编号
    int i_num;
    inode operator =(const inode &e){
        i_mode=e.i_mode;
        i_type=e.i_type;
        i_uid=e.i_uid;
        i_gid=e.i_gid;
        i_size=e.i_size;
        i_mtime=e.i_mtime;
        i_atime=e.i_atime;
        memcpy(i_blocks,e.i_blocks,N_BLOCKS* sizeof(int));
        i_blocks_count=e.i_blocks_count;
    }
    inode();
    inode(int inode_num,short file_type,unsigned int file_size,int blk_num);
    ~inode(){}
    void write();
    bool read(int inode_num);
};

/*目录项,64Bytes*/
struct dir_entry{
    //目录名
    char dname[NAME_LEN];
    //inode号
    int inode_num;

    dir_entry();
    void init(const char*name,int num){
        memcpy(dname,name, sizeof(name));
        inode_num=num;
    }
};

/*目录文件,1024Bytes*/
struct dir{
    //下级目录以及parent
    dir_entry dirs[DIR_LEN];
    unsigned int dir_count;
    dir();
    dir operator = (const dir &e){
        dir_count=e.dir_count;
        memcpy(dirs,e.dirs, sizeof(dirs));
    }
    void write(int blk_num);
    void read(int blk_num);
};
typedef dir directory[MAX_DIR_NUM];

/*普通文件,1024Bytes*/
struct data{
    char buf[BLOCK_SIZE];
    bool read(int blk_num);
    bool write(int blk_num);
};


#endif //LAB5_FS_H
