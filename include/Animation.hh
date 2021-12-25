#ifndef ANIMATION_HH_
#define ANIMATION_HH_
#include <list>
#include "Render.hh"
#include "Timer.hh"
#include "Event.hh"
#include <cstdio>

namespace easy {

	using EaseFunction = double(&)(double);

	constexpr double EaseLinear(double x) { return x; }
	constexpr double EaseInCubic(double x) { return x * x * x; }
	constexpr double EaseOutCubic(double x) { return 1 - EaseOutCubic(1 - x); }
	constexpr double EaseInOutCubic(double x) { return x < 0.5 ? 4 * x * x * x : 1 - 4 * (1 - x) * (1 - x) * (1 - x); }
	
	constexpr double EaseOutBounce(double x) {
		constexpr double n1 = 7.5625;
		constexpr double d1 = 2.75;
		if (x < 1 / d1) {
			return n1 * x * x;
		} else if (x < 2 / d1) {
			return n1 * (x -= 1.5 / d1) * x + 0.75;
		} else if (x < 2.5 / d1) {
			return n1 * (x -= 2.25 / d1) * x + 0.9375;
		} else {
			return n1 * (x -= 2.625 / d1) * x + 0.984375;
		}
	}
	constexpr double EaseInBounce(double x) { return 1 - EaseOutBounce(1 - x); }
	constexpr double EaseInOutBounce(double x) {
		return x < 0.5
			? (1 - EaseOutBounce(1 - 2 * x)) / 2
			: (1 + EaseOutBounce(2 * x - 1)) / 2;
	}

	struct IAnimation {
	private:

		using AnimationEventHandler = std::function<void(EventArgs)>;
		std::unordered_map<EventType, std::list<AnimationEventHandler>> listener;

		struct _Animation {
			IAnimation* owner = nullptr;
			TimerHandle handle;
			virtual void step() = 0;
			void stop() { *handle = false; }
			bool is_alive() const { return *handle; }
			static void static_step(_Animation* anim) { anim->step(); }
		};

		template<typename T, typename O>
		struct Animation : _Animation {
			O* ptr;
			T O::* prop;
			T from;
			T to;
			int curr;
			int total;
			EaseFunction f;

			Animation(IAnimation* owner, O* ptr, T O::* prop, T from, T to, int total, EaseFunction ease = EaseLinear) :
				ptr(ptr), prop(prop), from(from), to(to), curr(0), total(total), f(ease) { this->owner = owner; }

			void step() {
				double ratio = f(static_cast<double>(++curr) / total);
				assign_as_mixture(ptr->*prop, ratio, to, from);
				Renderer::Invalidated() = true;
				if (curr == total) {
					*_Animation::handle = false;
					bool finish_all = true;
					for (auto anim : _Animation::owner->anims) finish_all &= !(*anim->handle);
					if (finish_all) _Animation::owner->FinishAllAnimation(EventArgs { EventType::FinishAllAnimation });
				}
			}
		};
		
		std::list<_Animation*> anims;
		bool always_enable = true;

	protected:
		IAnimation() = default;

	public:

		_event_forwarder<EventType, EventType::StartAnyAnimation, AnimationEventHandler>
		StartAnyAnimation = { listener, always_enable };

		_event_forwarder<EventType, EventType::FinishAllAnimation, AnimationEventHandler>
		FinishAllAnimation = { listener, always_enable };

		template<typename T, typename O>
		void BeginAnimation(std::shared_ptr<O> object, T O::*prop, T from, T to, unsigned miliseconds, EaseFunction ease = EaseLinear, bool multiple = false, double FPS = 40) {
			unsigned total = static_cast<unsigned>(miliseconds * 0.001 * FPS);
			_Animation* anim = new Animation<T, O>(this, object.get(), prop, from, to, total, ease);
			for (auto itor = anims.begin(); itor != anims.end();) {
				if (!multiple) {
					Animation<T, O>* cast = dynamic_cast<Animation<T, O>*>(*itor);
					if (cast && cast->prop == prop) cast->stop();
				}
				if (!(*itor)->is_alive()) itor = anims.erase(itor);
				else itor++;
			}
			anim->handle = Timer::RecurrentInvoke(std::max(1, static_cast<int>(1000 / FPS)), 0, _Animation::static_step, anim);
			anims.push_back(anim);
			StartAnyAnimation(EventArgs { EventType::StartAnyAnimation });
		}

	};


}


#endif