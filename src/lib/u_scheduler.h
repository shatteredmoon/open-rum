#ifndef _U_SCHEDULER_H_
#define _U_SCHEDULER_H_

#include <u_script.h>

#include <queue>

// Used for scheduling timed callbacks to script object functions
class rumScheduler
{
public:

  // Perform callbacks on all objects that are less than the elapsed time
  void Dequeue();

  // Enqueue a callback to a script object's function
  bool Schedule( Sqrat::Object& i_sqWeakRef, Sqrat::Object& i_sqClosure, float i_fInterval, Sqrat::Object i_sqObject,
                 bool i_bHasObject );

  static void EnableScheduling( bool i_bEnable );
  static void Run();
  static void ScriptBind();
  static SQInteger Schedule_VM( HSQUIRRELVM i_pcVM );
  static void Shutdown();

private:
  // Flush all scheduled objects - called by Shutdown
  void Clear();

  struct CallbackObject
  {
    Sqrat::Object m_sqWeakInstance;
    Sqrat::Object m_sqWeakFunction;
    Sqrat::Object m_sqObject;

    double m_fExecutionTime{ 0.0 };
#ifdef _DEBUG
    double m_fScheduledTime{ 0.0 };
    float m_fInterval{ 0.0f };
#endif // _DEBUG

    rumScript::VMType m_eVMType{ rumScript::VM_INVALID };

    bool m_bHasObject{ false };

    ~CallbackObject()
    {
      m_sqWeakInstance.Release();
      m_sqWeakFunction.Release();
    }

    // For priority queue comparator
    bool operator<( const CallbackObject& i_rcCallbackObject ) const
    {
      return ( i_rcCallbackObject.m_fExecutionTime < m_fExecutionTime );
    }
  };

  std::priority_queue<CallbackObject, std::deque<CallbackObject>, std::less<CallbackObject>> m_qScheduledObjects;

  // Temporary location for items added during dequeue
  std::queue<CallbackObject> m_qCallbackObjects;

  // True when callback dequeue is running
  bool m_bRunning{ false };

  static bool s_bEnabled;
};

#endif // _U_SCHEDULER_H_
