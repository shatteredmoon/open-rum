#pragma once

#include <u_enum.h>

#include <sqrat.h>

#include <unordered_map>


// A classRegistry is a class that manages the various extended classes that exist in scripts. A base class extension
// such as rumCreature will have an entry in the registryContainer that is attached to a classRegistry that in turn
// holds a hash for every script that extends the base class rumCreature. There you'll find things such as
// Creature_Orc, Creature_Dragon, etc.
class rumClassRegistry
{
public:

  // Returns the script class indexed by the asset ID
  Sqrat::Object GetScriptClass( rumAssetID i_eAssetID ) const;

  bool RegisterScriptClass( rumAssetID i_eAssetID, const Sqrat::Object& i_sqClass );

  void UnregisterScriptClass( rumAssetID i_eAssetID )
  {
    m_hashScriptClasses.erase( i_eAssetID );
  }

private:

  typedef std::unordered_map<rumAssetID, Sqrat::Object> ClassContainer;
  ClassContainer m_hashScriptClasses;
};
