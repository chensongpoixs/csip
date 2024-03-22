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


	// ����__builtin_expect����������������֧Ԥ���׼ȷ��
#ifdef __GNUC__
//#  define SDK_LIKELY(x) __builtin_expect(!!(x), 1)          // x�ܿ���Ϊ��  
//#  define SDK_UNLIKELY(x) __builtin_expect(!!(x), 0)        // x�ܿ���Ϊ��
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
