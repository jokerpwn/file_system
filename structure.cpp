//
// Created by Wanning Pan on 2019/3/5.
//

#include "structure.h"


super_block::super_block():block_size(BLOCK_SIZE),inode_size(INODE_SIZE),first_free_inode(0),
                           first_free_block(0),blocks_count(BLOCK_NUM),inodes_count(INODE_NUM),
                           free_blocks_count(BLOCK_NUM),free_inodes_count(INODE_NUM),s_nfree(0)
{
    memset(block_bitmap,0, sizeof(block_bitmap));
    memset(inode_bitmap,0, sizeof(inode_bitmap));
    memset(user_map,0, sizeof(user_map));
}
bool super_block::read(){
    Disk.seekg(SUPER_BEGIN,Disk.beg);
    Disk.read((char *)this, SP_SIZE);

    ifstream user_file(USER_INF);
    if(!user_file.is_open())
        return false;

    for(int i=0;i<USER_NUM;i++){
        user_file>>user_map[i];
        user_file>>pwd_map[i];
    }
    user_file.close();
}

/**
 * 分配一个新的数据块
 * @param blk_num
 * @param inode_num
 * @return
 */
int super_block::get_new_block(){

    //无空闲块,则将进行i/o操作从磁盘读取新的空闲块
    if(!s_nfree){
        if(!free_blocks_count){
           first_free_block=-1;
            return -1;
        }
        else{
            //将空闲块充满
            for(int i=first_free_block;i<BLOCK_NUM&&s_nfree<FREE_STACK_NUM;i++)
            {
                if(!block_bitmap[i]){
                    s_free[s_nfree++]=i;
                    block_bitmap[i]=true;
                }
            }
            free_blocks_count-=s_nfree;
            first_free_block=s_free[0];
        }
    }

    return s_free[--s_nfree];
}
/**
 * 分配新的inode节点
 * @return
 */
int super_block::get_new_inode() {
    //无空闲块,则将进行i/o操作从磁盘读取新的空闲块
    if(!s_ninode){
        if(!free_inodes_count){
            first_free_inode=-1;
            return -1;
        }
        else{
            for(int i=first_free_inode;i<INODE_NUM&&s_ninode<FREE_STACK_NUM;i++){
                if(!inode_bitmap[i]){
                    s_inode[s_ninode++]=i;
                    inode_bitmap[i]=true;
                }
            }
            free_inodes_count-=s_nfree;
            first_free_inode=s_inode[0];
        }
    }

    return s_inode[--s_ninode];
}
/**
 *
 * @param del_block
 * @param del_node
 * @return
 */
bool super_block::delete_block(int del_block) {
    //已满
    if(s_nfree==FREE_STACK_NUM){
        while(s_nfree!=0){
            block_bitmap[s_free[--s_nfree]]=false;
            free_blocks_count++;
        }
    }
    //向内部填充
    s_free[s_nfree]=del_block;
    first_free_block=del_block;
    return true;
}

bool super_block::delete_inode(int inode_num) {
    if(inode_num>INODE_NUM||inode_num<0)
        return false;
    //已满
    if(s_ninode==FREE_STACK_NUM){
        while(s_ninode!=0){
            block_bitmap[s_inode[--s_ninode]]=false;
            free_inodes_count++;
        }
    }
    //向内部填充
    s_inode[s_ninode++]=inode_num;
    first_free_inode=inode_num;
    return true;
}
/**
 * 写回磁盘
 * @return
 */
bool super_block::write(){
    Disk.seekg(SUPER_BEGIN,Disk.beg);
    Disk.write((char*)this,SP_SIZE);
}

inode::inode():i_num(0),i_type(0),i_size(0),i_blocks_count(0),i_atime(0),i_mtime(0),
               i_gid(0),i_uid(0)
{
    memset(i_blocks,NULL,N_BLOCKS* sizeof(int));
}
inode::inode(int inode_num,short file_type, unsigned int file_size,int blk_num) {
    i_num=inode_num;
    i_type=file_type;
    i_size=file_size;
    i_blocks_count=1;
    i_blocks[0]=blk_num;

    time(&i_atime);
    time(&i_mtime);
}
void inode::write() {
    Disk.seekg(INODE_BEGIN+INODE_SIZE*i_num,Disk.beg);
    Disk.write((char *)this, INODE_SIZE);
}
bool inode::read(int inode_num){
    Disk.seekg(INODE_BEGIN+INODE_SIZE*inode_num,Disk.beg);
    Disk.read((char *)this,INODE_SIZE);
    return true;
}
void dir::write(int blk_num) {
    Disk.seekg(BLOCK_BEGIN+BLOCK_SIZE*blk_num,Disk.beg);
    Disk.write((char *)this, BLOCK_SIZE);
}
dir::dir():dir_count(2) {
    memset(dirs,NULL, sizeof(dirs));
}
dir_entry::dir_entry():inode_num(0){
    memset(dname,NULL, sizeof(dname));
}
bool data::read(int blk_num) {
    Disk.seekg(BLOCK_BEGIN+BLOCK_SIZE*blk_num,Disk.beg);
    Disk.read((char *)this,BLOCK_SIZE);
    return true;
}
bool data::write(int blk_num) {
    Disk.seekg(BLOCK_BEGIN+BLOCK_SIZE*blk_num,Disk.beg);
    Disk.write((char *)this,BLOCK_SIZE);
    return true;
}