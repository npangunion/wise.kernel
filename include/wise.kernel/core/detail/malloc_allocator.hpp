#pragma once

#include <stdlib.h>
#include <new>
#include <limits>

#undef min
#undef max

namespace wise {
	namespace kernel {

		/// A very simple malloc allocator for mem_tracker 
		template <class T>
		struct malloc_allocator : public std::allocator<T>
		{
			typedef size_t size_type;
			typedef ptrdiff_t difference_type;
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef T& reference;
			typedef const T& const_reference;
			typedef T value_type;

			template <class U> struct rebind { typedef malloc_allocator<U> other; };
			malloc_allocator() throw() {}
			malloc_allocator(const malloc_allocator&) throw() {}

			template <class U> malloc_allocator(const malloc_allocator<U>&) throw() {}

			~malloc_allocator() throw() {}

			pointer address(reference x) const { return &x; }
			const_pointer address(const_reference x) const { return &x; }

			pointer allocate(size_type s, void const* = 0)
			{
				if (0 == s)
					return nullptr;

				pointer temp = (pointer)std::malloc(s * sizeof(T));

				if (temp == nullptr)
				{
					throw std::bad_alloc();
				}

				return temp;
			}

			void deallocate(pointer p, size_type)
			{
				std::free(p);
			}

			size_type max_size() const throw()
			{
				return std::numeric_limits<size_t>::max() / sizeof(T);
			}

			void construct(pointer p, const T& val)
			{
				new((void*)p) T(val);
			}

			void destroy(pointer p)
			{
				p;
				p->~T();
			}

			bool operator==(const malloc_allocator& a) { return this == &a; }
			bool operator!=(const malloc_allocator& a) { return !operator==(a) };
		};

	} // kernel
} // wise
