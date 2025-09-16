#include <u_registry.h>

#include <u_assert.h>
#include <u_script.h>


Sqrat::Object rumClassRegistry::GetScriptClass( rumAssetID i_eAssetID ) const
{
  ClassContainer::const_iterator iter{ m_hashScriptClasses.find( i_eAssetID ) };
  return ( iter != m_hashScriptClasses.end() ? iter->second : Sqrat::Object() );
}


bool rumClassRegistry::RegisterScriptClass( rumAssetID i_eAssetID, const Sqrat::Object& i_sqClass )
{
  if( i_sqClass.GetType() != OT_CLASS )
  {
    return false;
  }

  const auto cPair{ m_hashScriptClasses.insert( std::make_pair( i_eAssetID, i_sqClass ) ) };
  rumAssert( cPair.second );
  return cPair.second;
}
