#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;     // 互斥锁
std::string delimiter = ":";

//Class template to implement node
template<typename K, typename V> 
class Node {

public:
    
    Node() {} 

    Node(K k, V v, int); 

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);
    
    // Linear array to hold pointers to next node of different level
    Node<K, V> **forward;

    int node_level;

private:
    K key;
    V value;
};

template<typename K, typename V> 
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level; 

    // 层是 0 - level 所以这里要加1  这里创建了 0 - level 层
    this->forward = new Node<K, V>*[level+1];
    
	// 初始化forward (初始化每一level的指针)
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));
};

template<typename K, typename V> 
Node<K, V>::~Node() {
    delete []forward;
};

template<typename K, typename V> 
K Node<K, V>::get_key() const {
    return key;
};

template<typename K, typename V> 
V Node<K, V>::get_value() const {
    return value;
};
template<typename K, typename V> 
void Node<K, V>::set_value(V value) {
    this->value=value;
};

// Class template for Skip list
template <typename K, typename V> 
class SkipList {

public: 
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:    
    // Maximum level of the skip list 
    int _max_level;

    // current level of skip list 
    int _skip_list_level;

    // pointer to header node 
    Node<K, V> *_header;

    // file operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // 元素的总个数
    int _element_count;
};

// create new node 
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    
    mtx.lock(); // 上锁
    Node<K, V> *current = this->_header;


    Node<K, V> *update[_max_level+1]; // 用于保存每一层刚好小于key的节点指针
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));  

    // 从上往下找到每一层小于key的节点指针
    for(int i = _skip_list_level; i >= 0; i--) {
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i]; 
        }
        update[i] = current;
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key.
    current = current->forward[0];

    // 新插入的key值已在跳表中
    if (current != NULL && current->get_key() == key) {
        //std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }


    if (current == NULL || current->get_key() != key ) {
        
        // 对于新加入的数据随机生成一个高度
        int random_level = get_random_level();

        // 随机生成的高度大于当前最大高度_skip_list_level
        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level+1; i < random_level+1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level; // 更新当前最高层数
        }

        // 创建一个新的节点用于插入数据
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        
        // 将新插入得节点连接到左右节点
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        //std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count ++; // 元素加1
    }
    mtx.unlock(); // 解锁
    return 0;
}

// 输出跳表 
template<typename K, typename V> 
void SkipList<K, V>::display_list() {

    std::cout << "\n*****Skip List*****"<<"\n"; 
    for (int i = 0; i <= _skip_list_level; i++) {
        Node<K, V> *node = this->_header->forward[i];  // 找到第几层
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" << node->get_value() << ";"; // 输出key-value
            node = node->forward[i]; // 走向下一个节点
        }
        std::cout << std::endl;
    }
}

// Dump data in memory to file 
template<typename K, typename V> 
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0]; 

    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return ;
}

// Load data from disk
template<typename K, typename V> 
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

// 获取当前跳表有多少元素
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

    if(!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// 删除一个元素
template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key) {

    mtx.lock(); // 上锁
    Node<K, V> *current = this->_header;  
    Node<K, V> *update[_max_level+1]; // 用于保存每一层刚好小于key的节点指针
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    // 找到每一层刚好小于key的节点指针
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    // 当前节点为需要删除的key
    if (current != NULL && current->get_key() == key) {
       
        // 从下往上改变指针的指向
        for (int i = 0; i <= _skip_list_level; i++) {

            // 左边已经不再指向需要删除的节点
            if (update[i]->forward[i] != current) 
                break;
            // 改变指向
            update[i]->forward[i] = current->forward[i];
        }

        // 更新当前最高的层数
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level --; 
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        _element_count --; // 元素减1
    }
    mtx.unlock(); // 解锁
    return;
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key) {

    //std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = _header;

    // 从上往下找，找到一个刚好小于key值的节点
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    // 指向下一个节点
    current = current->forward[0];

    // 如果相等的话，就找到了
    if (current and current->get_key() == key) {
        //std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }
    // 否则没找到
    //std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

/* 构造函数 */
template<typename K, typename V> 
SkipList<K, V>::SkipList(int max_level) {
    /* 初始化参数 */
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // 创建头节点
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};


/* 析构函数 */
template<typename K, typename V> 
SkipList<K, V>::~SkipList() {

    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    delete _header;
}

// 随机生成一个高度
template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};
