/*
**	********************************************************************************
**
**	File		: ps_common.hpp
**	Description	: 
**	Modify		: 2020/1/10		zhangqiang		Create the file
**	********************************************************************************
*/
#pragma once

namespace Zilu {
namespace StreamResolver {


	// 引入__builtin_expect函数来增加条件分支预测的准确性
#ifdef __GNUC__
//#  define SDK_LIKELY(x) __builtin_expect(!!(x), 1)          // x很可能为真  
//#  define SDK_UNLIKELY(x) __builtin_expect(!!(x), 0)        // x很可能为假
# define unlikely(p)   __builtin_expect(!!(p), 0)
#else /* __GNUC__ */
//#  define SDK_LIKELY(x) (x)
//#  define SDK_UNLIKELY(x) (x)

# define unlikely(p)    (p)
#endif /* __GNUC__ */


//# define unlikely(p)   __builtin_expect(!!(p), 0)

/** This block is scrambled */
#define BLOCK_FLAG_SCRAMBLED     0x0100

}
}
