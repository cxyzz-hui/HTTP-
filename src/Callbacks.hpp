#pragma
#ifndef CALLBACKS_HPP

//这里我们要给其他类型的智能指针和回调函数取别名，以便我们后期好分辨
#include <functional>
#include <memory>

class Buffer;
class Connection;

using ConnectionPtr = std::shared_ptr<Connection>;
using TimerCallback = std::function<void()>;

//声明事件回调类型
using CloseCallback = std::function<void(const ConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const ConnectionPtr&)>;
using ConnectionCallback = std::function<void(const ConnectionPtr&)>;

using MessageCallback = std::function<void(const ConnectionPtr& , Buffer*)>;


#endif