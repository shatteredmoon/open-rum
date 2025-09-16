#include <u_scheduler.h>

#include <u_assert.h>
#include <u_log.h>
#include <u_rum.h>

#if SCHEDULER_DEBUG
#include <u_object.h>
#endif

bool rumScheduler::s_bEnabled{ true };

// TODO: singleton?
rumScheduler g_cScheduler;


void rumScheduler::Dequeue()
{
  m_bRunning = true;

  const double fElapsedTime{ GetElapsedTime() };
  bool bContinue{ true };

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // See if scheduled time has been exceeded
  while( !m_qScheduledObjects.empty() && bContinue )
  {
    // Copy what's on top before popping it
    const CallbackObject cCallbackObject( m_qScheduledObjects.top() );
    m_qScheduledObjects.pop();

    // If multiple VMs are active, switch to the VM that scheduled the callback
    if( rumScript::GetCurrentVMType() != cCallbackObject.m_eVMType )
    {
      RUM_COUT_IFDEF( SCHEDULER_DEBUG, "Scheduler switching to VM id " << cCallbackObject.m_eVMType << '\n' );
      rumScript::SetCurrentVMType( cCallbackObject.m_eVMType );
    }

    if( fElapsedTime >= cCallbackObject.m_fExecutionTime )
    {
      Sqrat::Object sqInstance{ rumScript::GetWeakReferenceValue( cCallbackObject.m_sqWeakInstance ) };
      if( sqInstance.GetType() == OT_INSTANCE )
      {
        Sqrat::Object sqFunction{ rumScript::GetWeakReferenceValue( cCallbackObject.m_sqWeakFunction ) };
        if( sqFunction.GetType() == OT_CLOSURE )
        {
#if SCHEDULER_DEBUG
          std::string strObjectName{ rumScript::GetObjectName( pcVM, sqInstance ) };
          std::string strFunctionName{ rumScript::GetObjectName( pcVM, sqFunction ) };

          RUM_COUT( "De-queued " << strObjectName << "::" << strFunctionName << " t" <<
                    cCallbackObject.m_fExecutionTime << " = s" << cCallbackObject.m_fScheduledTime << " + i" <<
                    cCallbackObject.m_fInterval << '\n');
#endif // SCHEDULER_DEBUG

          Sqrat::Function sqFunc( pcVM, sqInstance, sqFunction );
          if( !sqFunc.IsNull() )
          {
            if( cCallbackObject.m_bHasObject )
            {
              // Call the script object's function with the extra object provided as a parameter
              if( cCallbackObject.m_sqObject.GetType() == OT_WEAKREF )
              {
                Sqrat::Object sqInstance{ rumScript::GetWeakReferenceValue( cCallbackObject.m_sqObject ) };
                if( sqInstance.GetType() == OT_INSTANCE )
                {
                  sqFunc.Execute( sqInstance );
                }
              }
              else
              {
                sqFunc.Execute( cCallbackObject.m_sqObject );
              }
            }
            else
            {
              // Call the script object's function
              sqFunc.Execute();
            }
          }

          if( Sqrat::Error::Occurred( pcVM ) )
          {
            std::string strError{ "Error: Call to script function failed\n" };

#if SCHEDULER_DEBUG
            strError += rumScript::GetObjectName( pcVM, sqInstance );
            strError += "::";
            strError += rumScript::GetObjectName( pcVM, sqFunction );
#endif // SCHEDULER_DEBUG

            strError += ": ";
            strError += Sqrat::Error::Message( pcVM );

            Logger::LogStandard( strError );
          }
        }
      }
    }
    else
    {
      // Have to push back
      m_qScheduledObjects.push( cCallbackObject );

      // All remaining items in queue have times occurring in the future - there is no need to check them
      bContinue = false;
    }
  }

  // See if any objects were newly queued during dequeue - prioritize them
  while( !m_qCallbackObjects.empty() )
  {
    CallbackObject& cCallbackObject{ m_qCallbackObjects.front() };
    m_qScheduledObjects.push( cCallbackObject );
    m_qCallbackObjects.pop();
  }

  m_bRunning = false;
}


// static
void rumScheduler::EnableScheduling( bool i_bEnable )
{
  s_bEnabled = i_bEnable;
}


// static
void rumScheduler::Run()
{
  g_cScheduler.Dequeue();
}


// static
void rumScheduler::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  Sqrat::RootTable( pcVM ).SquirrelFunc( "rumSchedule", &Schedule_VM );
}


// static
SQInteger rumScheduler::Schedule_VM( HSQUIRRELVM i_pcVM )
{
  if( !s_bEnabled )
  {
    return 0;
  }

  const SQInteger iTop{ sq_gettop( i_pcVM ) };
  SQInteger iParam{ 1 };

  HSQOBJECT sqTemp;

  // Get the calling object
  sq_getstackobj( i_pcVM, iParam++, &sqTemp );
  Sqrat::Object sqInstance( sqTemp );
  if( sqInstance.GetType() == OT_TABLE )
  {
    sq_getstackobj( i_pcVM, iParam++, &sqTemp );
    sqInstance = sqTemp;
  }

  if( sqInstance.GetType() != OT_INSTANCE )
  {
    std::string strError{ "Failed to schedule callback because an instance was not provided!\n\n" };
    strError += rumScript::GetCallstack( i_pcVM );
    rumAssertArgs( false, strError );
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return 0;
  }

  bool bScheduled{ false };

  sq_getstackobj( i_pcVM, iParam++, &sqTemp );
  Sqrat::Object sqClosure( sqTemp );
  if( sqClosure.GetType() != OT_CLOSURE )
  {
    std::string strError{ "Failed to schedule callback because a closure was not provided!\n\n" };
    strError += rumScript::GetCallstack( i_pcVM );
    rumAssertArgs( false, strError );
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return 0;
  }

  sq_getstackobj( i_pcVM, iParam++, &sqTemp );
  Sqrat::Object sqFloat( sqTemp );

  if( sqFloat.GetType() == OT_INTEGER )
  {
    rumScript::SetValue( sqFloat, sqFloat.Cast<float>() );
  }
  else if( sqFloat.GetType() != OT_FLOAT )
  {
    rumScript::SetValue( sqFloat, 0.f );
  }

  if( sqFloat.GetType() == OT_FLOAT )
  {
    Sqrat::Object sqWeakInstance;
    Sqrat::Object sqWeakClosure;

    if( sqInstance.GetType() == OT_INSTANCE )
    {
      sqWeakInstance = rumScript::GetWeakReference( sqInstance );
    }
    else
    {
      rumAssert( false );
      sqWeakInstance = sqInstance;
    }

    if( sqClosure.GetType() == OT_CLOSURE )
    {
      sqWeakClosure = rumScript::GetWeakReference( sqClosure );
    }
    else
    {
      rumAssert( false );
      sqWeakClosure = sqClosure;
    }

    bool bHasObject{ false };
    Sqrat::Object sqObject;

    if( iTop >= iParam )
    {
      sq_getstackobj( i_pcVM, iParam++, &sqTemp );
      sqObject = sqTemp;
      bHasObject = true;
    }

    bScheduled = g_cScheduler.Schedule( sqWeakInstance, sqWeakClosure, sqFloat.Cast<float>(), sqObject, bHasObject );
  }

  return bScheduled ? 1 : 0;
}


bool rumScheduler::Schedule( Sqrat::Object& i_sqWeakInstance, Sqrat::Object& i_sqWeakFunction, float i_fInterval,
                             Sqrat::Object i_sqObject, bool i_bHasObject )
{
  bool bScheduled{ false };

  if( ( i_sqWeakInstance.GetType() == OT_WEAKREF ) && ( i_sqWeakFunction.GetType() == OT_WEAKREF ) )
  {
    CallbackObject cCallbackObject;
    cCallbackObject.m_sqWeakInstance = i_sqWeakInstance;
    cCallbackObject.m_sqWeakFunction = i_sqWeakFunction;

    if( i_sqObject.GetType() == OT_INSTANCE )
    {
      // The scheduler shouldn't hold on to any references
      cCallbackObject.m_sqObject = rumScript::GetWeakReference( i_sqObject );
    }
    else
    {
      cCallbackObject.m_sqObject = i_sqObject;
    }
    cCallbackObject.m_bHasObject = i_bHasObject;
#ifdef _DEBUG
    cCallbackObject.m_fInterval = i_fInterval;
    cCallbackObject.m_fScheduledTime = GetElapsedTime();
#endif // _DEBUG

    // Multiple VMs may be active, so save the current VM id
    cCallbackObject.m_eVMType = rumScript::GetCurrentVMType();

    // This callback will occur at the current app time plus the specified time interval
    cCallbackObject.m_fExecutionTime = GetElapsedTime() + i_fInterval;

    if( m_bRunning )
    {
      // Priority queue is performing callbacks - we must add the object to a temporary location for now
      m_qCallbackObjects.push( cCallbackObject );
    }
    else
    {
      // Priority queue is not performing callbacks - safe to add object directly to the queue
      m_qScheduledObjects.push( cCallbackObject );
    }

#if SCHEDULER_DEBUG
    Sqrat::Object sqInstance{ rumScript::GetWeakReferenceValue( cCallbackObject.m_sqWeakInstance ) };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      Sqrat::Object sqFunction{ rumScript::GetWeakReferenceValue( cCallbackObject.m_sqWeakFunction ) };
      if( sqFunction.GetType() == OT_CLOSURE )
      {
        HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

        std::string strObjectName{ rumScript::GetObjectName( pcVM, sqInstance ) };
        std::string strFunctionName{ rumScript::GetObjectName( pcVM, sqFunction ) };

        RUM_COUT( "Scheduled " << strObjectName << "::" << strFunctionName << " interval " << i_fInterval << '\n' );
      }
    }
#endif // SCHEDULER_DEBUG

    bScheduled = true;
  }

  return bScheduled;
}


// static
void rumScheduler::Shutdown()
{
  RUM_COUT_IFDEF( SCHEDULER_DEBUG, "Scheduler shutting down\n" );
  g_cScheduler.Clear();
}


// This must be ran before the script system is released!
void rumScheduler::Clear()
{
  RUM_COUT_IFDEF( SCHEDULER_DEBUG, "Clearing all scheduled callbacks\n" );

  // Pop all pending items
  while( !m_qCallbackObjects.empty() )
  {
    m_qCallbackObjects.pop();
  }

  // Pop all callback objects
  while( !m_qScheduledObjects.empty() )
  {
    const CallbackObject& cCallbackObject{ m_qScheduledObjects.top() };

    // If multiple VMs are active, switch to the VM that scheduled the callback
    if( rumScript::GetCurrentVMType() != cCallbackObject.m_eVMType )
    {
      RUM_COUT_IFDEF( SCHEDULER_DEBUG, "Scheduler switching to VM id " << cCallbackObject.m_eVMType << '\n' );
      rumScript::SetCurrentVMType( cCallbackObject.m_eVMType );
    }

    m_qScheduledObjects.pop();
  }
}
