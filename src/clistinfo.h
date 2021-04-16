#ifndef _CLISTINFO_H_
#define _CLISTINFO_H_

#include <vector> // vector

template <typename TypeInfo>
class CListInfo : public std::vector<TypeInfo>
{
};

#endif
