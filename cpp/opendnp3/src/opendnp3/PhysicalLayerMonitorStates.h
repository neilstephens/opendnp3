#ifndef __PHYS_LAYER_MONITOR_STATES_H_
#define __PHYS_LAYER_MONITOR_STATES_H_

#include <opendnp3/Singleton.h>
#include <opendnp3/gen/ChannelState.h>
#include <openpal/Visibility.h>

#include "PhysicalLayerMonitor.h"


#define MACRO_MONITOR_SINGLETON(type, state, shuttingDown) \
	MACRO_NAME_SINGLETON_INSTANCE(type) \
	ChannelState GetState() const { return state; } \
	bool IsShuttingDown() const { return shuttingDown; }

namespace opendnp3
{

class PhysicalLayerMonitor;

/* --- Base classes --- */

class DLL_LOCAL IMonitorState
{
public:

	virtual void OnStartRequest(PhysicalLayerMonitor* apContext) = 0;
	virtual void OnStartOneRequest(PhysicalLayerMonitor* apContext) = 0;
	virtual void OnCloseRequest(PhysicalLayerMonitor* apContext) = 0;
	virtual void OnSuspendRequest(PhysicalLayerMonitor* apContext) = 0;
	virtual void OnShutdownRequest(PhysicalLayerMonitor* apContext) = 0;

	virtual void OnOpenTimeout(PhysicalLayerMonitor* apContext) = 0;
	virtual void OnOpenFailure(PhysicalLayerMonitor* apContext) = 0;
	virtual void OnLayerOpen(PhysicalLayerMonitor* apContext) = 0;
	virtual void OnLayerClose(PhysicalLayerMonitor* apContext) = 0;

	virtual ChannelState GetState() const = 0;
#ifndef OPENDNP3_STRIP_LOG_MESSAGES
	virtual std::string Name() const = 0;
#endif
	virtual bool IsShuttingDown() const = 0;

#ifndef OPENDNP3_STRIP_LOG_MESSAGES
	std::string ConvertToString();
#endif

};

class DLL_LOCAL MonitorStateActions
{
public:

	static void ChangeState(PhysicalLayerMonitor* apContext, IMonitorState* apState);
	static void StartOpenTimer(PhysicalLayerMonitor* apContext);
	static void CancelOpenTimer(PhysicalLayerMonitor* apContext);
	static void AsyncClose(PhysicalLayerMonitor* apContext);
	static void AsyncOpen(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL ExceptsOnLayerOpen : public virtual IMonitorState
{
public:
	void OnLayerOpen(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL NotOpening : public ExceptsOnLayerOpen
{
public:
	void OnOpenFailure(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL NotOpen : public virtual IMonitorState
{
public:
	void OnLayerClose(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL NotWaitingForTimer : public virtual IMonitorState
{
public:
	void OnOpenTimeout(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL IgnoresClose : public virtual IMonitorState
{
public:
	void OnCloseRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL IgnoresSuspend : public virtual IMonitorState
{
public:
	void OnSuspendRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL StartsOnClose : public virtual IMonitorState
{
public:
	void OnLayerClose(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL IgnoresShutdown : public virtual IMonitorState
{
public:
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL IgnoresStart : public virtual IMonitorState
{
public:
	void OnStartRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL IgnoresStartOne : public virtual IMonitorState
{
public:
	void OnStartOneRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL OpenFailureCausesWait : public virtual IMonitorState
{
public:
	void OnOpenFailure(PhysicalLayerMonitor* apContext);
};

template <class T>
class DLL_LOCAL OpenFailureGoesToState : public virtual IMonitorState
{
public:
	void OnOpenFailure(PhysicalLayerMonitor* apContext);
};

// disable "inherits via dominance warning", it's erroneous b/c base
// class is pure virtual and G++ correctly deduces this and doesn't care
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4250)
#endif

class DLL_LOCAL MonitorStateWaitingBase : public virtual IMonitorState,
	private NotOpening, private NotOpen, private IgnoresClose
{
	void OnSuspendRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};

/* --- Concrete classes --- */

class DLL_LOCAL MonitorStateShutdown : public virtual IMonitorState,
	private NotOpening,
	private NotOpen,
	private NotWaitingForTimer,
	private IgnoresClose,
	private IgnoresStart,
	private IgnoresStartOne,
	private IgnoresShutdown,
	private IgnoresSuspend
{
	MACRO_MONITOR_SINGLETON(MonitorStateShutdown, ChannelState::SHUTDOWN, true);
};

class DLL_LOCAL MonitorStateSuspendedBase : public virtual IMonitorState,
	private NotOpening,
	private NotOpen,
	private NotWaitingForTimer,
	private IgnoresClose,
	private IgnoresSuspend
{
	void OnStartRequest(PhysicalLayerMonitor* apContext);
	void OnStartOneRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};


class DLL_LOCAL MonitorStateSuspended : public MonitorStateSuspendedBase
{
	MACRO_MONITOR_SINGLETON(MonitorStateSuspended, ChannelState::CLOSED, false);
};

class DLL_LOCAL MonitorStateInit : public MonitorStateSuspendedBase
{
	MACRO_MONITOR_SINGLETON(MonitorStateInit, ChannelState::CLOSED, false);
};

class DLL_LOCAL MonitorStateOpeningBase : public virtual IMonitorState,
	private NotOpen,
	private NotWaitingForTimer
{
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
	void OnSuspendRequest(PhysicalLayerMonitor* apContext);
};


class DLL_LOCAL MonitorStateOpening : public MonitorStateOpeningBase,
	private OpenFailureCausesWait,
	private IgnoresStart
{
	MACRO_MONITOR_SINGLETON(MonitorStateOpening, ChannelState::OPENING, false);

	void OnStartOneRequest(PhysicalLayerMonitor* apContext);
	void OnCloseRequest(PhysicalLayerMonitor* apContext);
	void OnLayerOpen(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateOpeningOne : public MonitorStateOpeningBase,
	private IgnoresStartOne
{
	MACRO_MONITOR_SINGLETON(MonitorStateOpeningOne, ChannelState::OPENING, false);

	void OnOpenFailure(PhysicalLayerMonitor* apContext);
	void OnStartRequest(PhysicalLayerMonitor* apContext);
	void OnCloseRequest(PhysicalLayerMonitor* apContext);
	void OnLayerOpen(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateOpeningClosing : public virtual IMonitorState,
	private NotOpen,
	private NotWaitingForTimer,
	private ExceptsOnLayerOpen,
	private OpenFailureCausesWait,
	private IgnoresStart,
	private IgnoresClose
{
	MACRO_MONITOR_SINGLETON(MonitorStateOpeningClosing, ChannelState::OPENING, false);

	void OnStartOneRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
	void OnSuspendRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateOpeningStopping : public virtual IMonitorState,
	private NotOpen,
	private NotWaitingForTimer,
	private ExceptsOnLayerOpen,
	private OpenFailureGoesToState<MonitorStateShutdown>,
	private IgnoresStart,
	private IgnoresStartOne,
	private IgnoresClose,
	private IgnoresSuspend,
	private IgnoresShutdown
{
	MACRO_MONITOR_SINGLETON(MonitorStateOpeningStopping, ChannelState::OPENING, true);
};

class DLL_LOCAL MonitorStateOpeningSuspending : public virtual IMonitorState,
	private NotOpen,
	private NotWaitingForTimer,
	private ExceptsOnLayerOpen,
	private OpenFailureGoesToState<MonitorStateSuspended>,
	private IgnoresClose,
	private IgnoresStartOne,
	private IgnoresSuspend
{
	MACRO_MONITOR_SINGLETON(MonitorStateOpeningSuspending, ChannelState::OPENING, false);

	void OnStartRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateOpen : public virtual IMonitorState,
	private NotOpening,
	private NotWaitingForTimer,
	private IgnoresStart,
	private StartsOnClose
{
	MACRO_MONITOR_SINGLETON(MonitorStateOpen, ChannelState::OPEN, false);

	void OnStartOneRequest(PhysicalLayerMonitor* apContext);
	void OnCloseRequest(PhysicalLayerMonitor* apContext);
	void OnSuspendRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateOpenOne : public virtual IMonitorState,
	private NotOpening,
	private NotWaitingForTimer,
	private IgnoresStartOne
{
	MACRO_MONITOR_SINGLETON(MonitorStateOpenOne, ChannelState::OPEN, false);

	void OnLayerClose(PhysicalLayerMonitor* apContext);
	void OnStartRequest(PhysicalLayerMonitor* apContext);
	void OnCloseRequest(PhysicalLayerMonitor* apContext);
	void OnSuspendRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateWaiting : public MonitorStateWaitingBase, private IgnoresStart
{
	MACRO_MONITOR_SINGLETON(MonitorStateWaiting, ChannelState::WAITING, false);

	void OnStartOneRequest(PhysicalLayerMonitor* apContext);
	void OnOpenTimeout(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateWaitingOne : public MonitorStateWaitingBase, private IgnoresStartOne
{
	MACRO_MONITOR_SINGLETON(MonitorStateWaitingOne, ChannelState::WAITING, false);

	void OnStartRequest(PhysicalLayerMonitor* apContext);
	void OnOpenTimeout(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateClosing : public virtual IMonitorState,
	private NotOpening, private NotWaitingForTimer, private IgnoresStart, private IgnoresClose, private StartsOnClose
{
	MACRO_MONITOR_SINGLETON(MonitorStateClosing, ChannelState::CLOSED, false);

	void OnStartOneRequest(PhysicalLayerMonitor* apContext);
	void OnSuspendRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateSuspending : public virtual IMonitorState,
	private NotOpening, private NotWaitingForTimer, private IgnoresClose, private IgnoresSuspend, private IgnoresStartOne
{
	MACRO_MONITOR_SINGLETON(MonitorStateSuspending, ChannelState::CLOSED, false);

	void OnLayerClose(PhysicalLayerMonitor* apContext);
	void OnStartRequest(PhysicalLayerMonitor* apContext);
	void OnShutdownRequest(PhysicalLayerMonitor* apContext);
};

class DLL_LOCAL MonitorStateShutingDown : public virtual IMonitorState,
	private NotOpening,
	private NotWaitingForTimer,
	private IgnoresStart,
	private IgnoresStartOne,
	private IgnoresClose,
	private IgnoresShutdown,
	private IgnoresSuspend
{
	MACRO_MONITOR_SINGLETON(MonitorStateShutingDown, ChannelState::CLOSED, true);

	void OnLayerClose(PhysicalLayerMonitor* apContext);
};

template <class T>
void DLL_LOCAL OpenFailureGoesToState<T>::OnOpenFailure(PhysicalLayerMonitor* apContext)
{
	MonitorStateActions::ChangeState(apContext, T::Inst());
}

#ifdef WIN32
#pragma warning(pop)
#endif

}

#endif

