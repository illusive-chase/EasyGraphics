#ifndef TIMER_HH_
#define TIMER_HH_

#include <chrono>
#include <queue>
#include <memory>

namespace easy {

	using TimerHandle = std::shared_ptr<bool>;

	struct Timer {
	private:

		template<typename T>
		struct pointer_less {
			bool operator()(T a, T b) const { return *a < *b; }
		};

		struct _TimeTask {
			std::chrono::steady_clock::time_point next, target;
			std::chrono::milliseconds step;
			TimerHandle enabled;
			_TimeTask(const std::chrono::steady_clock::time_point& next,
					  const std::chrono::steady_clock::time_point& target,
					  const std::chrono::milliseconds& step
			) :next(next), target(target), step(step), enabled(new bool{true}) {}

			bool operator <(const _TimeTask& rhs) const {
				return next > rhs.next;
			}

			virtual bool execute(const std::chrono::steady_clock::time_point& now) { return true; };
		};

		template<typename T>
		struct TimeTask : _TimeTask {
			T t;
			TimeTask(const std::chrono::steady_clock::time_point& next,
					 const std::chrono::steady_clock::time_point& target,
					 const std::chrono::milliseconds& step, const T& t
			) : _TimeTask(next, target, step), t(t) {}

			bool execute(const std::chrono::steady_clock::time_point& now) {
				if (!*enabled) return true;
				*enabled = next < target;
				while (next <= now && *enabled) {
					t();
					next += step;
					*enabled &= next < target;
				}
				return next >= target;
			}
		};

		template<typename T>
		static TimeTask<T>* MakeTimeTask(const std::chrono::steady_clock::time_point& next,
										 const std::chrono::steady_clock::time_point& target,
										 const std::chrono::milliseconds& step, const T& t) {
			return new TimeTask<T>(next, target, step, t);
		}


		std::priority_queue<_TimeTask*, std::vector<_TimeTask*>, pointer_less<_TimeTask*>> q;

		Timer() {}

		static Timer& instance() {
			static Timer timer;
			return timer;
		}

	public:

		static void Sync() {
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			while (!instance().q.empty() && instance().q.top()->next <= now) {
				auto* top = instance().q.top();
				bool fin = top->execute(now);
				instance().q.pop();
				if (fin) delete top;
				else instance().q.push(top);
			}
		}

		
		template<typename F, typename ... T>
		static TimerHandle DelayInvoke(unsigned delay, F&& f, T&&... args) {
			auto closure = [=]() { f(args...); };
			auto now = std::chrono::steady_clock::now();
			auto* task = MakeTimeTask(now + std::chrono::milliseconds(delay), now + std::chrono::milliseconds(delay + 1), std::chrono::hours(24 * 36500), closure);
			instance().q.push(task);
			return task->enabled;
		}

		template<typename F, typename ... T>
		static TimerHandle RecurrentInvoke(unsigned interval, unsigned times, F&& f, T&&... args) {
			auto closure = [=]() { f(args...); };
			auto now = std::chrono::steady_clock::now();
			auto step = std::chrono::milliseconds(interval);
			auto target = now + step * times;
			if (!times) target = now + std::chrono::hours(24 * 36500);
			auto* task = MakeTimeTask(now, target, step, closure);
			instance().q.push(task);
			return task->enabled;
		}

	};


}


#endif