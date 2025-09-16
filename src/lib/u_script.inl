#ifndef _U_SCRIPT_INL_
#define _U_SCRIPT_INL_

namespace rumScript
{
  template<typename T, typename... TArgs>
  std::pair<bool, T> EvalOptionalFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, T i_vDefault,
                                       TArgs... i_TArgs )
  {
    bool bExecuted{ true };
    T TVal{ i_vDefault };

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Function sqFunc( i_sqObject, i_strFunc.c_str() );
    if( !sqFunc.IsNull() )
    {
      TVal = *sqFunc.Evaluate<T>( i_TArgs... );
      if( Sqrat::Error::Occurred( pcVM ) )
      {
        std::string strError{ "Error: Call to script function " };
        strError += i_strFunc;
        strError += "() failed: ";
        strError += Sqrat::Error::Message( pcVM );
        Logger::LogStandard( strError );
        bExecuted = false;
      }
    }
    else
    {
      bExecuted = false;
      Sqrat::Error::Clear( pcVM );
    }

    return std::make_pair( bExecuted, TVal );
  }


  template<typename T, typename... TArgs>
  std::pair<bool, T> EvalRequiredFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, T i_vDefault,
                                       TArgs... i_TArgs )
  {
    bool bExecuted{ true };
    T TVal{ i_vDefault };

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Function sqFunc( i_sqObject, i_strFunc.c_str() );
    if( !sqFunc.IsNull() )
    {
      TVal = *sqFunc.Evaluate<T>( i_TArgs... );
    }

    if( Sqrat::Error::Occurred( pcVM ) )
    {
      bExecuted = false;

      std::string strError{ "Error: Call to script function " };
      strError += i_strFunc;
      strError += "() failed: ";
      strError += Sqrat::Error::Message( pcVM );
      Logger::LogStandard( strError );
    }


    return std::make_pair( bExecuted, TVal );
  }


  template<typename... TArgs>
  bool ExecOptionalFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, TArgs... i_TArgs )
  {
    bool bExecuted{ false };

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Function sqFunc( i_sqObject, i_strFunc.c_str() );
    if( !sqFunc.IsNull() )
    {
      sqFunc.Execute( i_TArgs... );

      if( Sqrat::Error::Occurred( pcVM ) )
      {
        std::string strError{ "Error: Call to script function " };
        strError += i_strFunc;
        strError += "() failed: ";
        strError += Sqrat::Error::Message( pcVM );
        Logger::LogStandard( strError );
      }
      else
      {
        bExecuted = true;
      }
    }
    else
    {
      Sqrat::Error::Clear( pcVM );
    }

    return bExecuted;
  }


  template<typename... TArgs>
  bool ExecRequiredFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, TArgs... i_TArgs )
  {
    bool bExecuted{ false };

    // Handle the script callback - scripts are required to provide this function
    Sqrat::Function sqFunc( i_sqObject, i_strFunc.c_str() );
    if( !sqFunc.IsNull() )
    {
      // Execute can accept up to templated args!
      sqFunc.Execute( i_TArgs... );
      bExecuted = true;
    }

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    if( Sqrat::Error::Occurred( pcVM ) )
    {
      std::string strError{ "Error: Call to script function " };
      strError += i_strFunc;
      strError += "() failed: ";
      strError += Sqrat::Error::Message( pcVM );
      Logger::LogStandard( strError );

      bExecuted = false;
    }

    return bExecuted;
  }


  template<typename T>
  Sqrat::Object SetValue( Sqrat::Object& o_sqObject, const T& i_TVal )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    Sqrat::PushVar( pcVM, i_TVal );
    o_sqObject = AttachToStackObject( -1 );
    sq_poptop( pcVM );
    return o_sqObject;
  }
} // namespace rumScript

#endif //_U_SCRIPT_H_
