#ifndef EVENT_HH_
#define EVENT_HH_

#include <memory>
#include <unordered_map>
#include <functional>
#include <list>
#include "LinearType.hh"


namespace easy {

	template<typename EnumType, EnumType E, typename HandlerType>
	struct _event_forwarder {
		std::unordered_map<EnumType, std::list<HandlerType>>& listener;
		bool& enable;

		void operator +=(HandlerType handler) {
			listener[E].push_back(handler);
		}

		template<typename ...T>
		void operator()(T&&... args) {
			if (enable)
				for (HandlerType& handler : listener[E])
					handler(std::forward<T>(args)...);
		}
	};

	enum class EventType {
		EnterFrame,
		Drag,
		Click,
		StartAnyAnimation,
		FinishAllAnimation
		//VisibleChanged,
		//EnableChanged,
	};

	struct EventArgs {
		EventType type;
	};

	struct MouseEventArgs {
		EventType type;
		Pos pos;
		Pos offset;
	};

	
	template<typename T>
	struct IEvent {
	private:
		using EventHandler = std::function<void(T, EventArgs)>;
		using MouseEventHandler = std::function<void(T, MouseEventArgs)>;
		std::unordered_map<EventType, std::list<EventHandler>> listener;
		std::unordered_map<EventType, std::list<MouseEventHandler>> mouse_listener;

	protected:
		IEvent() = default;

	public:


		bool Enable = true;
		_event_forwarder<EventType, EventType::EnterFrame, EventHandler> EnterFrame = { listener, Enable };
		_event_forwarder<EventType, EventType::Drag, MouseEventHandler> Drag = { mouse_listener, Enable };
		_event_forwarder<EventType, EventType::Click, MouseEventHandler> Click = { mouse_listener, Enable };
		//_event_forwarder<EventType, EventType::VisibleChanged, EventHandler> VisibleChanged = { listener, Enable };
		//_event_forwarder<EventType, EventType::EnableChanged, EventHandler> EnableChanged = { listener, Enable };

		void AddEventListener(EventType event_type, EventHandler handler) {
			listener[event_type].emplace_back(handler);
		}

		void AddEventListener(EventType event_type, MouseEventHandler handler) {
			mouse_listener[event_type].emplace_back(handler);
		}

		void RemoveEventListener(EventType event_type, EventHandler handler) {
			listener[event_type].remove(handler);
		}

		void RemoveEventListener(EventType event_type, MouseEventHandler handler) {
			mouse_listener[event_type].remove(handler);
		}

		void RemoveAllListener(EventType event_type) {
			listener[event_type].clear();
			mouse_listener[event_type].clear();
		}

		void RemoveAllListener() {
			listener.clear();
			mouse_listener.clear();
		}

	};

}

#endif