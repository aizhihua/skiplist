#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {

    SkipList<std::string, std::string> skipList(6);
	skipList.insert_element("1", "测试"); 
	skipList.insert_element("3", "测试3"); 
	skipList.insert_element("7", "测试7"); 
	skipList.insert_element("8", "测试8"); 
	skipList.insert_element("9", "测试9"); 
	skipList.insert_element("19", "测试19"); 
	skipList.insert_element("19", "测试20"); 

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
