#pragma once

#include "atomic.h"

namespace crstl
{
	template<typename T>
	void intrusive_ptr_add_ref(T* ptr)
	{
		ptr->add_ref();
	}

	template<typename T>
	void intrusive_ptr_release(T* ptr)
	{
		int32_t currentRef = ptr->release_ref();

		if (currentRef == 0)
		{
			ptr->delete_callback();
		}
	}

	template<typename T>
	class intrusive_ptr
	{
	public:

		intrusive_ptr() = default;

		intrusive_ptr(T* ptr) : m_ptr(ptr)
		{
			if (ptr)
			{
				intrusive_ptr_add_ref(ptr);
			}
		}

		~intrusive_ptr()
		{
			if (m_ptr)
			{
				intrusive_ptr_release(m_ptr);
			}
		}

		intrusive_ptr& operator = (T* ptr)
		{
			if (m_ptr != ptr)
			{
				T* const ptr_temp = m_ptr;

				// Add a reference to the new pointer
				if (ptr)
				{
					intrusive_ptr_add_ref(ptr);
				}

				// Assign to member pointer
				m_ptr = ptr;

				// Release reference from the old pointer we used to hold on to
				if (ptr_temp)
				{
					intrusive_ptr_release(ptr_temp);
				}
			}

			return *this;
		}

		T* get() const { return m_ptr; }

		T* operator ->() const
		{
			return m_ptr;
		}

		typedef T* (intrusive_ptr<T>::*boolean)() const;

		operator boolean() const
		{
			// Return anything that isn't easily castable but is guaranteed to be non-null, such as the get function pointer
			return m_ptr ? &intrusive_ptr<T>::get : nullptr;
		}

		bool operator!() const
		{
			return (m_ptr == nullptr);
		}

	private:

		T* m_ptr = nullptr;
	};

	class intrusive_ptr_interface
	{
	public:

		virtual ~intrusive_ptr_interface() {}

		virtual int32_t add_ref()
		{
			m_refcount++;
			return m_refcount;
		}

		virtual int32_t release_ref()
		{
			m_refcount--;

			if (m_refcount == 0)
			{
				// Delete the thing
			}

			return m_refcount;
		}

		virtual void delete_callback()
		{
			delete this;
		}

		crstl::atomic<int32_t> m_refcount;
	};
};