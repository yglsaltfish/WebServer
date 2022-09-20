#ifndef BASE_NONCOPYABLE_H
#define BASE_NONCOPYABLE_H

//防止指针复制导致继承的字类将父类删除
namespace Zlearn
{

class noncopyable
{
 public:
  noncopyable(const noncopyable&) = delete;
  void operator= (const noncopyable&) = delete;

  protected:
   noncopyable() = default;
   ~noncopyable() = default;
};
    
}


#endif