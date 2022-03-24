#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H


/**
 * @brief 一个基类，禁用拷贝构造函数 禁止拷贝
 * 
 */
class uncopyable
{
protected:
    uncopyable(){}
    ~uncopyable(){}
private:
    uncopyable(const uncopyable&);
    uncopyable& operator=(const uncopyable&);
};

#endif