template<typename... TArgs>
void rumClientControl::CallScriptFunc( Sqrat::Object& i_sqFunc, TArgs... i_TArgs )
{
  if( i_sqFunc.GetType() != OT_NULL )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Function sqFunc( pcVM, i_sqFunc.GetObject(), i_sqFunc.GetObject() );
    sqFunc.Execute( i_TArgs... );

    if( Sqrat::Error::Occurred( pcVM ) )
    {
      std::string strError{ "Error: Call to script function failed: " };
      strError += Sqrat::Error::Message( pcVM );
      Logger::LogStandard( strError );
    }
  }
}
