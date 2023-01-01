#pragma once

namespace crstl
{
	template<typename T, size_t N = 1>
	class array
	{
	public:

		constexpr T* begin() { return m_data; }
		constexpr const T* begin() const { return m_data; }
		constexpr const T* cbegin() const { return m_data; }

		constexpr T* end() { return m_data + N; }
		constexpr const T* end() const { return m_data + N; }
		constexpr const T* cend() const { return m_data + N; }

		constexpr bool empty() const { return N == 0; }
		constexpr size_t size() const { return N; }
		constexpr size_t max_size() const { return N; }

		constexpr T* data() { return m_data; }
		constexpr T* const data() const { return m_data; }

		constexpr T& operator[](size_t i) { return m_data[i]; }
		constexpr const T& operator[](size_t i) const { return m_data[i]; }

		constexpr T& at(size_t i) { return m_data[i]; }
		constexpr const T& at(size_t i) const { return m_data[i]; }

		// Public to allow braced initialization
		T m_data[N ? N : 1];
	};
}