/* ************************************************************************
> File Name:     main.cpp
> Author:        程序员Carl
> 微信公众号:    代码随想录
> Created Time:  Sun Dec  2 20:21:41 2018
> Description:   
 ************************************************************************/
#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {

    SkipList<std::string, std::string> skipList(6);
	skipList.insert_element("1", "学"); 
	skipList.insert_element("3", "算法"); 
	skipList.insert_element("7", "认准"); 
	skipList.insert_element("8", "微信公众号：代码随想录"); 
	skipList.insert_element("9", "学习"); 
	skipList.insert_element("19", "算法不迷路"); 
	skipList.insert_element("19", "赶快关注吧你会发现详见很晚！"); 

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();

    // skipList.load_file();

    std::cout << skipList.search_element("9") << std::endl;
    skipList.search_element("18");

    std::cout << "display_list" << std::endl;
    skipList.display_list();

    //skipList.delete_element("3");
    //skipList.delete_element("7");

    //std::cout << "skipList size:" << skipList.size() << std::endl;

    //skipList.display_list();
}
