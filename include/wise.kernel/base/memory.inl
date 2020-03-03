#pragma once

#include "memory.hpp"

template <typename Ty, typename... Args>
inline
Ty* wise_new(Args&&... args)
{
	Ty* p = new Ty(std::forward<Args>(args)...);

#if WISE_TRACK_MEMORY
	wise::mem_tracker::inst().on_alloc(
		reinterpret_cast<uintptr_t>(p), sizeof(Ty), typeid(Ty).name()
	);
#endif

	return p;
}

template <typename Ty>
inline 
Ty* wise_new_array(std::size_t count)
{
	WISE_ASSERT(count > 0);

	Ty* p = new Ty[count];

#if WISE_TRACK_MEMORY

	std::string name("array of "); name.append(typeid(Ty).name());

	wise::mem_tracker::inst().on_alloc(
		reinterpret_cast<uintptr_t>(p), sizeof(Ty) * count, name.c_str()
	);
#endif
	
	return p;
}

template <typename Ty, typename... Args>
inline
std::shared_ptr<Ty> wise_shared(Args&&... args)
{
	auto p = std::make_shared<Ty>(std::forward<Args>(args)...);

#if WISE_TRACK_MEMORY
	Ty* addr = p.get();

	// XXX: 32비트 빌드에서 확인. 윈도우 플래폼에서만 지원하도록 설정

	const int hack_shared_ptr_addr = 0x10;

	wise::mem_tracker::inst().on_alloc(
		reinterpret_cast<uintptr_t>(addr) - hack_shared_ptr_addr, 
		sizeof(Ty), typeid(Ty).name()
	);
#endif
	return p;
}


template <typename Ty, typename... Args>
inline
std::unique_ptr<Ty> wise_unique(Args&&... args)
{
	auto p = std::make_unique<Ty>(std::forward<Args>(args)...);

#if WISE_TRACK_MEMORY
	Ty* addr = p.get();

	wise::mem_tracker::inst().on_alloc(
		reinterpret_cast<uintptr_t>(addr), 
		sizeof(Ty), typeid(Ty).name()
	);
#endif

	return p;
}