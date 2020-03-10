#pragma once 

#include "concurrent_queue.hpp"
#include "macros.hpp"

namespace wise {
	namespace kernel {

		// simple object pool. thread-safe with a cocurrent queue
		template <typename Obj, typename Allocator = std::allocator<Obj>>
		class object_pool
		{
		public:
			using ptr = Obj*;

			// internal use only. automate free
			class ref
			{
			public:
				ref() {}
				ref(ptr p, object_pool* pool);
				~ref();

				ptr operator->();
				ptr operator *();
				ptr get() { return operator*(); }

				ref(const ref& rhs) = delete;
				ref& operator=(const ref& rhs) = delete;

			private:
				void destroy();

			private:
				object_pool* pool_ = nullptr;
				ptr p_ = nullptr;
			};

			// wrapper shared_ptr to automate free and 
			// to make pointer derefence easier
			class shared_ref : public std::shared_ptr<ref>
			{
			public: 
				shared_ref() 
					: std::shared_ptr<ref>()
				{
				}

				shared_ref(ref* r)
					: std::shared_ptr<ref>(r)
				{
				}

				Obj* operator->() const noexcept 
				{
					return get()->get();
				}
			};

		public:
			object_pool(const std::string& name) 
				: name_(name)
			{
			}

			~object_pool()
			{
				cleanup();
			}

			template<class... Types>
			shared_ref create(Types&&... args)
			{
				ptr obj = alloc();

				::new ((void*)obj) Obj(std::forward<Types>(args)...);

				return shared_ref(new ref(obj, this));
			}

			void destroy(ptr obj)
			{
				(obj)->~Obj();

				dealloc(obj);
			}

			std::size_t free_count() const
			{
				return pool_free_count_;
			}

			std::size_t alloc_count() const
			{
				return pool_alloc_count_;
			}

		private:
			ptr alloc()
			{
				ptr obj;

				if (q_.pop(obj))
				{
					WISE_EXPECT(pool_free_count_ > 0);
					--pool_free_count_;

					++pool_alloc_count_;
					WISE_ENSURE(pool_alloc_count_ > 0);

					return obj;
				}

				ptr p = allocator_.allocate(1);

				++pool_alloc_count_;
				WISE_ENSURE(pool_alloc_count_ > 0);

				return p;
			}

			void dealloc(ptr obj)
			{
				WISE_EXPECT(pool_alloc_count_ > 0);
				--pool_alloc_count_;

				q_.push(obj);

				++pool_free_count_;
				WISE_ENSURE(pool_free_count_ > 0);
			}

			void cleanup()
			{
				ptr obj;

				if (pool_alloc_count_ > 0)
				{
					WISE_ERROR(
						"object_pool: {}. {} objects not freed.", 
						name_.c_str(), 
						pool_alloc_count_
					);
				}

				while (q_.pop(obj))
				{
					allocator_.deallocate(obj, 1);
				}
			}

		private:
			using queue = concurrent_queue<ptr>;

		private:
			std::string name_;
			queue q_;
			Allocator allocator_;
			std::atomic<std::size_t> pool_free_count_ = 0;
			std::atomic<std::size_t> pool_alloc_count_ = 0;
		};

		template <typename Obj, typename Allocator>
		object_pool<Obj, Allocator>::ref::ref(ptr p, object_pool* pool)
			: p_(p)
			, pool_(pool)
		{
			WISE_ASSERT(pool_);
			WISE_ASSERT(p_);
		}

		template <typename Obj, typename Allocator>
		object_pool<Obj, Allocator>::ref::~ref()
		{
			if (pool_ && p_)
			{
				pool_->destroy(p_);
			}
		}

		template <typename Obj, typename Allocator>
		typename object_pool<Obj, Allocator>::ptr
			object_pool<Obj, Allocator>::ref::operator->()
		{
			return p_;
		}

		template <typename Obj, typename Allocator>
		typename object_pool<Obj, Allocator>::ptr
			object_pool<Obj, Allocator>::ref::operator*()
		{
			return p_;
		}

		template <typename Obj, typename Allocator>
		void object_pool<Obj, Allocator>::ref::destroy()
		{
			if (pool_ && p_)
			{
				pool_->destroy(p_);
				p_ = 0;
			}
		}
	} // kernel
} // wise
