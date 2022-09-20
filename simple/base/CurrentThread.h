#ifndef BASE_CURRENTTHREAD_H
#define BASE_CURRENTTHREAD_H

#include "base/noncopyable.h"
#include "base/Types.h"

namespace Zlearn
{
namespace CurrentThread
{
 extern __thread int t_cachedTId;
 extern __thread char t_tidString[32];


}

}



#endif