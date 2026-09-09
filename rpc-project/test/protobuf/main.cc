#include <iostream>
#include <string>
#include "test.pb.h"

using namespace fixbug;

void fun1 () {
     // 序列化
    LoginRequest req;
    req.set_name("zhangsan");
    req.set_pwd("123456");

    std::string req_str;
    if (req.SerializeToString(&req_str)) {
        std::cout << "serialize success" << std::endl;
        std::cout << "req_str: " << req_str << std::endl;
    } else {
        std::cout << "serialize error" << std::endl;
    }
    // 反序列化
    LoginRequest req2;
    if (req2.ParseFromString(req_str)) {
        std::cout << "parse success" << std::endl;
        std::cout << "name: " << req2.name() << std::endl;
        std::cout << "pwd: " << req2.pwd() << std::endl;
    } else {
        std::cout << "parse error" << std::endl;
    }
}

void fun2 () {
    GetFriendListsResponse res;
    ErrorInfo *error = res.mutable_errinfo();
    error->set_errcode(0);
    error->set_errmsg("success");

    User *user1 = res.add_friendlist();
    user1->set_name("zhangsan");
    user1->set_age(18);
    user1->set_sex(User::MALE);
    User *user2 = res.add_friendlist();
    user2->set_name("lisi");
    user2->set_age(20);
    user2->set_sex(User::FEMALE);

    std::cout << res.friendlist_size() << std::endl;
}

int main() {
    fun2();
    return 0;
}
