#include "file_system.h"

int main() {
    file_system test;
    char  usr[NAME_LEN],pwd[NAME_LEN];
    while(true){
        cout<<"please input the user name"<<endl;
        cin>>usr;
        cout<<"please input the password"<<endl;
        cin>>pwd;

        if(test.login(usr,pwd)){
            cout<<"successfully login"<<endl;
            break;
        }
        else{
            cout<<"fail to login"<<endl;
        }
    }
    getchar();
    cout<<"whether load last record?(y/n)"<<endl;
    string ans;
    getline(cin,ans);
    bool tag=true;
    if(ans=="y"||ans=="Y"){
        tag=false;
    }
    test.run(tag);

    return 0;
}