


#ifndef _C_SAFE_QUEUE_H_
#define _C_SAFE_QUEUE_H_
#include <mutex>

#include <queue>


namespace chen {



	template<typename T>
	class csafe_queue
	{
	private:
		typedef std::mutex							clock_type;
		typedef std::lock_guard<clock_type>			clock_guard;
	public:
		csafe_queue() = default;
		~csafe_queue() = default;
		void push(const T&  t)
		{
			clock_guard lock(m_lock);
			m_queue.push(t);
		}

		bool pop(T& out)
		{
			clock_guard lock(m_lock);
			if (m_queue.empty())
			{
				return false;
			}
			out = m_queue.front();
			m_queue.pop();
			return true;
		}
	private:
		clock_type					m_lock;
		std::queue<T>				m_queue;
	};
}

#endif // _C_SAFE_QUEUE_H_