//
// Created by Wanning Pan on 2019/3/5.
//

#include "file_system.h"
const char *get_name(const char* path) {
    int len = strlen(path);
    int i;
    for (i =len-1; i > 0 && path[i-1] != '/'; i--);
    return (path+i);
}

fstream Disk;

file_system::file_system():i_root(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block()){
    memset(buffer,0, sizeof(buffer));
    sp.read();
    Disk.open(DISK, fstream::in | fstream::out | fstream::binary);
    if(Disk.is_open())
        cout << "文件已经打开" << endl;
    else
        cout << "文件没有打开" << endl;
}
file_system::~file_system(){
    Disk.close();
}
/**
 * 将inode_num对应数据读入inode节点和目录，如果不是目录文件则返回false
 * @param inode_num
 * @param cur_node
 * @param cur_dir
 * @return
 */
bool file_system::open_dir(int inode_num, inode *cur_node, directory *cur_dir) {
    //读取inode
    Disk.seekg(INODE_BEGIN + inode_num * INODE_SIZE, Disk.beg);
    Disk.read((char *) cur_node, INODE_SIZE);

    if (cur_node->i_type == DIR_TYPE) {
        //读取目录文件信息
        int i;
        for (i = 0; i < cur_node->i_blocks_count; i++) {
            Disk.seekg(BLOCK_BEGIN + cur_node->i_blocks[i] * BLOCK_SIZE, Disk.beg);
            Disk.read((char *) cur_dir[i], BLOCK_SIZE);
        }
        return true;
    } else//非目录文件
        return false;
}
bool file_system::open_dir(inode *cur_node, directory *cur_dir) {
    if (cur_node->i_type == DIR_TYPE) {
        //读取目录文件信息
        int i;
        for (i = 0; i < cur_node->i_blocks_count; i++) {
            Disk.seekg(BLOCK_BEGIN + cur_node->i_blocks[i] * BLOCK_SIZE, Disk.beg);
            Disk.read((char *) cur_dir[i], BLOCK_SIZE);
        }
        return true;
    } else//非目录文件
        return false;
}
/**
 * 将数据写回磁盘
 * @param inode_num
 * @return
 */
bool file_system::close_dir(const inode *i_w, directory *dir_w) {

    Disk.seekg(INODE_BEGIN + INODE_SIZE * i_w->i_num, Disk.beg);
    Disk.write((char *) i_w, INODE_SIZE);
    for(int i=0;i<i_w->i_blocks_count;i++){
        Disk.seekg(BLOCK_BEGIN + BLOCK_SIZE * i_w->i_blocks[i], Disk.beg);
        Disk.write((char *) dir_w[i], BLOCK_SIZE);
    }

    sp.write();

    return true;
}

/**
 *路径解析,buffer变为父母目录
 * @param dir_name
 * @return
 */
int file_system::namei(const char *dir_name) {
    int inode_num = i_cur->i_num;
    int i, j = 0;

    memcpy(buffer, *dir_cur, sizeof(buffer));

    inode cur_node = *i_cur;
    //路径解析
    for (i = 0; dir_name[i]; i++) {

        if (dir_name[i] == '/') {
            //根目录
            if (i==0) {
                inode_num = 1;
                buffer[0] =root[0];
                cur_node = i_root;
            }
                //非根目录，需要根据工作目录解析
            else {
                bool tag = false;
                for (int l = 0; l < cur_node.i_blocks_count&&!tag; l++) {
                    for (int k = 0; k < buffer[l].dir_count; k++) {
                        if (!strncmp(buffer[l].dirs[k].dname, dir_name + j, i-j)) {
                            //读取信息加载进buffer
                            inode_num = buffer[l].dirs[k].inode_num;
                            open_dir(inode_num, &cur_node, &buffer);
                            tag = true;
                            break;
                        }
                    }
                }

                //查找失败
                if (!tag) {
                    inode_num = -1;
                    break;
                }
            }
            j = i+1;
        }
    }
    if(inode_num!=-1){
        bool tag=false;

        for (int l = 0; l < cur_node.i_blocks_count&&!tag; l++) {
            for (int k = 0; k < buffer[l].dir_count; k++) {
                if (!strncmp(buffer[l].dirs[k].dname, dir_name + j, i-j)) {
                    //读取信息inode_num
                    inode_num = buffer[l].dirs[k].inode_num;
                    tag = true;
                    break;
                }
            }
        }
        if(!tag)
            inode_num=-1;
    }


    return inode_num;
}
/**
 * 同时获取父母的namei
 * @param dir_name
 * @param i_par
 * @return
 */
int file_system::namei(const char *dir_name, int &i_par) {
    int inode_num = i_cur->i_num;
    int i, j = 0;

    memcpy(buffer, *dir_cur, sizeof(buffer));

    inode cur_node = *i_cur;
    i_par = inode_num;
    //路径解析
    for (i = 0; dir_name[i]; i++) {

        if (dir_name[i] == '/') {
            //根目录
            if (i==0) {
                inode_num = 1;
                buffer[0] =root[0];
                cur_node = i_root;
                i_par = inode_num;
            }
                //非根目录，需要根据工作目录解析
            else {
                bool tag = false;
                for (int l = 0; l < cur_node.i_blocks_count&&!tag; l++) {
                    for (int k = 0; k < buffer[l].dir_count; k++) {
                        if (!strncmp(buffer[l].dirs[k].dname, dir_name + j, i-j)) {
                            //读取信息加载进buffer
                            inode_num = buffer[l].dirs[k].inode_num;
                            open_dir(inode_num, &cur_node, &buffer);
                            i_par = inode_num;
                            tag = true;
                            break;
                        }
                    }
                }

                //查找失败
                if (!tag) {
                    i_par=-1;
                    inode_num = -1;
                    break;
                }
            }
            j = i+1;
        }
    }
    if(inode_num!=-1){
        bool tag=false;

        for (int l = 0; l < cur_node.i_blocks_count&&!tag; l++) {
            for (int k = 0; k < buffer[l].dir_count; k++) {
                if (!strncmp(buffer[l].dirs[k].dname, dir_name + j, i-j)) {
                    //读取信息inode_num
                    inode_num = buffer[l].dirs[k].inode_num;
                    tag = true;
                    break;
                }
            }
        }
        if(!tag)
            inode_num=-1;
    }
    return inode_num;
}
/**
 * 获取位置的namei
 * @param dir_name
 * @param i_par
 * @param loc
 * @return
 */
int file_system::namei(const char *dir_name, int &i_par, pair<int, int> &loc) {
    int inode_num = i_cur->i_num;
    int i,j = 0;

    memcpy(buffer, *dir_cur, sizeof(buffer));

    inode cur_node =* i_cur;
    i_par = inode_num;
    //路径解析
    for (i = 0; dir_name[i]; i++) {

        if (dir_name[i] == '/') {
            //根目录
            if (i==0) {
                inode_num = 1;
                buffer[0] =root[0];
                cur_node = i_root;
                i_par = inode_num;
            }
                //非根目录，需要根据工作目录解析
            else {
                bool tag = false;
                for (int l = 0; l < cur_node.i_blocks_count&&!tag; l++) {
                    for (int k = 0; k < buffer[l].dir_count; k++) {
                        if (!strncmp(buffer[l].dirs[k].dname, dir_name + j, i-j)) {
                            //读取信息加载进buffer
                            inode_num = buffer[l].dirs[k].inode_num;
                            open_dir(inode_num, &cur_node, &buffer);
                            i_par = inode_num;
                            tag = true;
                            break;
                        }
                    }
                }

                //查找失败
                if (!tag) {
                    i_par=-1;
                    inode_num = -1;
                    break;
                }
            }
            j = i+1;
        }
    }
    if(inode_num!=-1){
        bool tag=false;
        int l,k;
        for (l = 0; l < cur_node.i_blocks_count&&!tag; l++) {
            for (k = 0; k < buffer[l].dir_count; k++) {
                if (!strncmp(buffer[l].dirs[k].dname, dir_name + j, i-j)){
                    //读取信息inode_num
                    inode_num = buffer[l].dirs[k].inode_num;
                    tag = true;
                    break;
                }
            }
        }
        if(!tag)
            inode_num=-1;
        else
        {
            loc.first=l-1;
            loc.second=k;
        }
    }
    return inode_num;
}
/**
 * 列出工作目录下所有文件名,非递归
 * @return
 */
bool file_system::list_dir() {

    //目录文件不同块迭代
    for (int i = 0; i < i_cur->i_blocks_count; i++) {
        //目录文件不同目录项迭代
        for (int j = 0; j < dir_cur[i]->dir_count; j++) {
            char *name = dir_cur[i]->dirs[j].dname;
            if (!strcmp(name, "..") || !strcmp(name, "."))
                continue;
            inode i_tmp;
            i_tmp.read(dir_cur[i]->dirs[j].inode_num);
            char modify_time[NAME_LEN];
            strftime(modify_time, sizeof(modify_time),"%Y-%m-%d %H:%M:%S",localtime(&i_tmp.i_mtime));

            cout << dir_cur[i]->dirs[j].dname << " "<<modify_time<<endl;
        }
    }
}

/**
 * 根据输入的文件名转移工作目录
 * @param dir_name
 * @return
 */
bool file_system::chdir(const char *dir_name) {

    //seek first
    int inode_num;

    //为父目录或本身目录
    if (!strcmp(dir_name, "..")) {
        inode_num = dir_cur[0]->dirs[1].inode_num;
        open_dir(inode_num,i_cur,dir_cur);

        return true;
    }
    else if (!strcmp(dir_name, ".")) {
        return true;
    }
    else if ((inode_num=namei(dir_name)) == -1) {
        cout << "no such file or directory exist!" << endl;
        return false;
    }
    else {
        //打开对应目录
        open_dir(inode_num,i_cur,dir_cur);
        return true;
    }
}

/**
 * 根据输入的目录名在相对位置创建目录
 * @param dir_name
 * @return
 */
bool file_system::make_dir(const char *dir_name) {
    int par_num;

    if (namei(dir_name,par_num) != -1) {
        cout << "already exists the directory!" << endl;
        return false;
    }
    else if(par_num==-1)
    {
        cout<<"parent driectory doesn't exists"<<endl;
        return false;
    }

    //创建inode
    int new_inode_num = sp.get_new_inode();
    int new_blk_num = sp.get_new_block();
    sp.write();
    inode new_inode(new_inode_num, DIR_TYPE, 0, new_blk_num);
    //写入磁盘
    new_inode.write();

    //建立目录结构
    dir new_dir;
    new_dir.dirs[0].init(".", new_inode_num);
    new_dir.dirs[1].init("..",par_num);
    //写入磁盘
    new_dir.write(new_blk_num);

    //修改父母目录结构,写入磁盘
    inode i_par;
    i_par.read(par_num);
    time(&i_par.i_mtime);
    int count = buffer[i_par.i_blocks_count-1].dir_count++;
    if (count == DIR_LEN) {
        i_par.i_blocks_count++;
        count = 0;
    }
    i_par.write();
    buffer[i_par.i_blocks_count-1].dirs[count].init(get_name(dir_name), new_inode_num);
    buffer[i_par.i_blocks_count-1].write(i_par.i_blocks[i_par.i_blocks_count-1]);

    if(i_par.i_num==i_cur->i_num){
        *i_cur=i_par;
        *dir_cur[i_cur->i_blocks_count-1]=buffer[i_par.i_blocks_count-1];
    }
    return true;
}

/**
 * 删除目录
 * @param dir_name
 * @return
 */
bool file_system::remove_dir(const char *dir_name) {
    if (!strcmp(dir_name, "..") || !strcmp(dir_name, ".")) {
        cout << "\".\" and \"..\" may not be removed" << endl;
        return false;
    }
    int par_num;
    pair<int,int>loc;
    int inode_num = namei(dir_name,par_num,loc);
    if (inode_num == -1) {
        cout << "no such file or directory exsits!" << endl;
        return false;
    }
    inode i_dir,i_par;
    i_dir.read(inode_num);

    del_inode(i_dir);

    //修改父母目录结构
    i_par.read(par_num);
    int count=--buffer[i_par.i_blocks_count-1].dir_count;
    buffer[loc.first].dirs[loc.second]=buffer[i_par.i_blocks_count-1].dirs[count];

    if (buffer[i_par.i_blocks_count-1].dir_count == 1) {
        i_par.i_blocks_count--;
    }
    time(&i_par.i_mtime);

    close_dir(&i_par,&buffer);
    if(i_par.i_num==i_cur->i_num){
        *i_cur=i_par;
        *dir_cur[i_cur->i_blocks_count-1]=buffer[i_par.i_blocks_count-1];
    }
}

/**
 * 用于递归删除
 * @param cur_node
 * @param depth
 * @return success or not
 */
bool file_system::del_inode(inode &cur_node) {
    if (cur_node.i_type == FILE_TYPE) {
        sp.delete_inode(cur_node.i_num);

        for(int i=0;i<cur_node.i_blocks_count;i++)
            sp.delete_block(cur_node.i_blocks[i]);
    }
    else {

        //bitmap中置空
        sp.delete_inode(cur_node.i_num);
        for (int i = 0; i < cur_node.i_blocks_count; i++)
            sp.delete_block(cur_node.i_blocks[i]);

        //读取当前层目录数据，需要分配空间
        directory new_dir;
        open_dir(&cur_node, &new_dir);

        for (int i = 0; i < cur_node.i_blocks_count; i++) {
            //从子目录开始
            for (int j = 2; j < new_dir[i].dir_count; j++) {
                cur_node.read(new_dir[i].dirs[j].inode_num);
                del_inode(cur_node);
            }
        }

    }
}

/**
 * 修改时间，若文件名不存在则创建
 * @param file_name
 * @return
 */
bool file_system::touch(const char *file_name) {

    int inode_num;
    inode i_par;
    //不存在该文件，创建
    if ((inode_num=namei(file_name, i_par.i_num))== -1) {

        open_dir(i_par.i_num,&i_par,&buffer);
        int top=i_par.i_blocks_count-1;
        int count=buffer[top].dir_count;
        if(count==DIR_LEN){
            i_par.i_blocks_count++;
            top++;
            count=0;
            i_par.write();
        }
        buffer[top].dir_count++;

        const char *name=get_name(file_name);
        int new_node_num=sp.get_new_inode();
        int new_blk_num=sp.get_new_block();
        inode i_new(new_node_num,FILE_TYPE,0,new_blk_num);

        buffer[top].dirs[count].init(name,i_new.i_num);
        sp.write();
        i_new.write();
        buffer[top].write(i_par.i_blocks[top]);

        if(i_par.i_num==i_cur->i_num){
            *i_cur=i_par;
            *dir_cur[i_cur->i_blocks_count-1]=buffer[i_par.i_blocks_count-1];
        }
        return true;
    }
    else{//修改时间

        time_t tt;
        time(&tt);
        inode i_file;
        i_file.read(inode_num);
        i_file.i_mtime=tt;
        i_file.write();
        return false;
    }
}
/**
 * 实现MV指令，移动文件到指定目录
 * @param from_name
 * @param to_name
 * @return
 */
bool file_system::move_file(const char *from_name, const char *to_name) {
    int par_num,inode_num,dir_num;
    pair<int,int> loc;
    inode i_par,i_dir;

    if((inode_num=namei(from_name,par_num,loc))==-1){
        cout<<"no such file: "<<from_name<<" exsits!"<<endl;
        return false;
    }
    else{
        //删除父母目录中对应目录项
        i_par.read(par_num);
        int count=(--buffer[loc.first].dir_count);
        buffer[loc.first].dirs[loc.second]=buffer[loc.first].dirs[count];
        close_dir(&i_par,&buffer);

        if(i_par.i_num==i_cur->i_num){
            *i_cur=i_par;
            *dir_cur[i_cur->i_blocks_count-1]=buffer[i_par.i_blocks_count-1];
        }

        //打开修改目的目录项
        dir_num=namei(to_name);
        open_dir(dir_num,&i_dir,&buffer);
        //目的目录不正确
        if(dir_num==-1||i_dir.i_type==FILE_TYPE){
            cout<<"no such directory:"<<to_name<<"exists"<<endl;
            return false;
        }
        else{
            const char* name=get_name(from_name);
            unsigned int &top=i_dir.i_blocks_count;

            //目的目录下增加待添加源文件
            int tail=buffer[top-1].dir_count;
            if(tail==DIR_LEN)
            {
                tail=0;
                top++;
            }
            buffer[top-1].dir_count++;
            buffer[top-1].dirs[tail].init(name,inode_num);
            buffer[top-1].write(i_dir.i_blocks[top-1]);

            if(i_dir.i_num==i_cur->i_num){
                *i_cur=i_dir;
                *dir_cur[top-1]=buffer[top-1];
            }
        }
    }

    return true;
}
/**
 * 将文件信息显示至控制台上
 * @param file_name
 * @return
 */
bool file_system::cat(const char *file_name) {
    int inode_num=-1;
    inode i_file;

    if((inode_num=namei(file_name))==-1){
        cout<<"no such file exsits!"<<endl;
        return false;
    }
    else{
        i_file.read(inode_num);
        if(i_file.i_type!=FILE_TYPE)
        {
            cout<<"It is a directory, fail to read"<<endl;
            return false;
        }
        cout<<"output the file content"<<endl;
        data file;
        for(int i=0;i<i_file.i_blocks_count;i++){
            file.read(i_file.i_blocks[i]);
            cout<<file.buf<<endl;
        }
    }
}
/**
 * 将字符串重定向写入文件
 * @param input
 * @param file_name
 * @return
 */
bool file_system::echo(const char* input,const char*file_name) {
    int inode_num;
    if((inode_num=namei(file_name))==-1){
        cout<<"no such file exsits!"<<endl;
        return false;
    }
    inode i_file;
    i_file.read(inode_num);
    if(i_file.i_type==DIR_TYPE){
        cout<<"It is a directory, fail to write."<<endl;
        return false;
    }
    data file;
    strcpy(file.buf,input);
    file.write(i_file.i_blocks[0]);
    return true;
}
/**
 * 运行函数
 * @return
 */
bool file_system::format_fs() {
    //对根目录初始化
    cout<<"申请inode..."<<endl;

    inode i_bin(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block());
    inode i_etc(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block());
    inode i_home(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block());
    inode i_dev(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block());
    inode i_boot(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block());
    inode i_usr(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block());
    inode i_pwn(sp.get_new_inode(),DIR_TYPE,0,sp.get_new_block());
    cout<<"申请inode结束"<<endl;

    cout<<"建立目录..."<<endl;
    root[0].dir_count=8;
    root[0].dirs[0].init(".",i_root.i_num);
    root[0].dirs[1].init("..",i_root.i_num);
    root[0].dirs[2].init("bin",i_bin.i_num);
    root[0].dirs[3].init("etc",i_etc.i_num);
    root[0].dirs[4].init("home",i_home.i_num);
    root[0].dirs[5].init("dev",i_dev.i_num);
    root[0].dirs[6].init("boot",i_boot.i_num);
    root[0].dirs[7].init("usr",i_usr.i_num);

    dir dir_bin;
    dir_bin.dirs[0].init(".",i_bin.i_num);
    dir_bin.dirs[1].init("..",i_root.i_num);

    dir dir_etc;
    dir_etc.dirs[0].init(".",i_etc.i_num);
    dir_etc.dirs[1].init("..",i_root.i_num);

    dir dir_home;
    dir_home.dirs[0].init(".",i_home.i_num);
    dir_home.dirs[1].init("..",i_root.i_num);
    dir_home.dir_count++;
    dir_home.dirs[2].init("pwn",i_pwn.i_num);

    dir dir_dev;
    dir_dev.dirs[0].init(".",i_dev.i_num);
    dir_dev.dirs[1].init("..",i_root.i_num);

    dir dir_boot;
    dir_boot.dirs[0].init(".",i_boot.i_num);
    dir_boot.dirs[1].init("..",i_root.i_num);

    dir dir_usr;
    dir_usr.dirs[0].init(".",i_usr.i_num);
    dir_usr.dirs[1].init("..",i_root.i_num);

    dir dir_pwn;
    dir_pwn.dirs[0].init(".",i_pwn.i_num);
    dir_pwn.dirs[1].init("..",i_home.i_num);

    cout<<"目录创建结束"<<endl;
    dir_cur=&root;
    i_cur=&i_root;

    //写回磁盘
    i_bin.write();
    i_boot.write();
    i_dev.write();
    i_etc.write();
    i_home.write();
    i_usr.write();
    i_pwn.write();
    i_root.write();
    dir_bin.write(i_bin.i_blocks[0]);
    dir_boot.write(i_boot.i_blocks[0]);
    dir_dev.write(i_dev.i_blocks[0]);
    dir_etc.write(i_etc.i_blocks[0]);
    dir_home.write(i_home.i_blocks[0]);
    dir_pwn.write(i_pwn.i_blocks[0]);
    dir_usr.write(i_usr.i_blocks[0]);
    root[0].write(i_root.i_blocks[0]);
}
/**
 *
 * @param tag
 */
void file_system::run(bool tag) {

    if(tag)
        format_fs();
    else{
        //读取上一次结果
        i_root.read(ROOT_NODE);
        dir_cur=&root;
        i_cur=&i_root;
        open_dir(i_root.i_num,i_cur,dir_cur);
    }
    cout<<"**************welcome****************"<<endl;
    cout<<"following command support"<<endl;
    cout<<"* ls"<<endl;
    cout<<"* cd"<<endl;
    cout<<"* mv"<<endl;
    cout<<"* touch"<<endl;
    cout<<"* rm"<<endl;
    cout<<"* cat"<<endl;
    cout<<"* mkdir"<<endl;
    cout<<"* exit"<<endl;
    vector<string> commands;
    string input,str;
    while(true){
        cout<<">>";
        getline(cin,input);
        stringstream split_str(input);
        while(split_str>>str)
            commands.push_back(str);
        if(commands.size()==0)
            continue;
        else if(commands[0]=="ls"){
            list_dir();
        }
        else if(commands[0]=="cd"){
            chdir(commands[1].c_str());
        }
        else if(commands[0]=="mkdir"){
            make_dir(commands[1].c_str());
        }
        else if(commands[0]=="touch"){
            bool res=touch(commands[1].c_str());
            if(res){
                cout<<"no such file:"<<commands[1]<<",created"<<endl;
            }
            else
                cout<<commands[1]<<"exists,modify the time information"<<endl;
        }
        else if(commands[0]=="cat"){
            cat(commands[1].c_str());
        }
        else if(commands[0]=="rm"){
            remove_dir(commands[1].c_str());
        }
        else if(commands[0]=="mv"){
            if(commands.size()<3)
            {
                cout<<"invalid input"<<endl;
                continue;
            }
            move_file(commands[1].c_str(),commands[2].c_str());
        }
        else if(commands[0]=="echo"){
            if(commands.size()<3)
            {
                cout<<"invalid input"<<endl;
                continue;
            }
            echo(commands[1].c_str(),commands[2].c_str());
        }
        else if(commands[0]=="exit"){
            cout<<"succesfully quit!"<<endl;
            break;
        }
        else{
            cout<<"unsurpport input!"<<endl;
        }
        commands.clear();

    }
}
/**
 * 多用户登陆
 * @param user_name
 * @param pwd
 * @return
 */
bool file_system::login(const char *user_name, const char *pwd) {
    int i;
    for(i=0;i<USER_NUM;i++){
        if(!strcmp(user_name,sp.user_map[i])){
            if(!strcmp(pwd,sp.pwd_map[i])){
                cur_gid=sp.user_gid[i];
                break;
            }
        }
    }
    if(i==USER_NUM)
        return false;

    return true;
}