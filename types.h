#ifndef __YDX_TYPES_H__
#define __YDX_TYPES_H__

namespace ydx
{

template<typename To, typename From>
inline To implicit_cast(From const &f)
{
	return f;
}

template<typename To, typename From>     // use like this: down_cast<T*>(foo);
inline To down_cast(From* f)                     // so we only accept pointers
{
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false)
  {
    implicit_cast<From*, To>(0);
  }

  return static_cast<To>(f);
}



}


#endif
