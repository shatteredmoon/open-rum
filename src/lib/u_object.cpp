#include <u_object.h>

#include <u_broadcast.h>
#include <u_custom.h>
#include <u_db.h>
#include <u_graphic.h>
#include <u_inventory.h>
#include <u_log.h>
#include <u_map.h>
#include <u_pawn.h>
#include <u_property_asset.h>
#include <u_resource.h>
#include <u_rum.h>
#include <u_sound.h>
#include <u_timer.h>

rumTimer timerCleanup;

// Static member initialization
rumGameObject::ScriptObjectContainer rumGameObject::s_hashScriptObjects;

#ifdef WIN32
#undef GetObject
#endif // WIN32


rumGameObject::~rumGameObject()
{
#if MEMORY_DEBUG
  std::string strInfo{ "Destroying " };
  strInfo += GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( GetGameID() );
  strInfo += "]";
  Logger::LogStandard( strInfo );
#endif // MEMORY_DEBUG

  FreeInternal();

  // If these asserts fail, chances are this object is being destroyed unexpectedly before Free() has been called.
  // The culprit is probably a refcount problem with the script object
  rumAssert( m_hashProperties.size() == 0 );
  //rumAssert( m_sqInstance.IsNull() );
  m_pcAsset = nullptr;
}


// static
Sqrat::Object rumGameObject::Create( rumAssetID i_eAssetID, Sqrat::Table i_sqParams, rumUniqueID i_uiGameID )
{
  rumAsset* pcAsset{ rumAsset::Fetch( i_eAssetID ) };
  if( !pcAsset )
  {
    std::string strError{ "Failed to create an object from unknown asset [" };
    strError += rumStringUtils::ToHexString( i_eAssetID );
    strError += "]";
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return Sqrat::Object();
  }

#if MEMORY_DEBUG
  std::string strInfo{ "Creating asset " };
  strInfo += pcAsset->GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString( i_eAssetID );
  strInfo += "]";
  if( i_uiGameID != INVALID_GAME_ID )
  {
    strInfo += "[";
    strInfo += rumStringUtils::ToHexString64( i_uiGameID );
    strInfo += "]";
  }
  Logger::LogStandard( strInfo );
#endif // MEMORY_DEBUG

  Sqrat::Object sqInstance;

  Sqrat::Object sqClass{ pcAsset->GetScriptClass() };
  if( sqClass.GetType() == OT_CLASS )
  {
    sqInstance = rumScript::CreateInstance( sqClass, i_sqParams );
  }

  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumGameObject* pcObject{ sqInstance.Cast<rumGameObject*>() };
    if( pcObject )
    {
      pcObject->m_sqInstance = rumScript::GetWeakReference( sqInstance );
      pcObject->m_pcAsset = pcAsset;
      pcObject->AllocateGameID( i_uiGameID );
      pcObject->OnCreated();
    }
    else
    {
      std::string strError{ "Failed to fetch script instance user pointer for " };
      strError += pcAsset->GetName();
      Logger::LogStandard( strError, Logger::LOG_ERROR );
      return Sqrat::Object();
    }
  }
  else
  {
    std::string strError{ "Failed to create a script instance for asset " };
    strError += pcAsset->GetName();
    strError += " [";
    strError += rumStringUtils::ToHexString( pcAsset->GetAssetID() );
    strError += "]";

    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return sqInstance;
  }

  return sqInstance;
}


// static
Sqrat::Object rumGameObject::Create( const Sqrat::Object& i_sqClass, Sqrat::Table i_sqParams, rumUniqueID i_uiGameID )
{
  Sqrat::Object sqInstance;

  if( i_sqClass.GetType() == OT_CLASS )
  {
    sqInstance = rumScript::CreateInstance( i_sqClass, i_sqParams );
  }
  else if( i_sqClass.GetType() == OT_INSTANCE )
  {
    sqInstance = i_sqClass;
  }

  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumGameObject* pcObject{ sqInstance.Cast<rumGameObject*>() };
    rumAssert( pcObject );
    if( pcObject )
    {
      pcObject->m_sqInstance = rumScript::GetWeakReference( sqInstance );
      pcObject->AllocateGameID( i_uiGameID );
      pcObject->OnCreated();
    }
    else
    {
      std::string strError = "Failed to fetch script instance user pointer for new rumGameObject";
      Logger::LogStandard( strError, Logger::LOG_ERROR );
    }
  }
  else
  {
    std::string strError = "Failed to create a script instance";
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return sqInstance;
  }

  return sqInstance;
}


// static
rumGameObject* rumGameObject::Fetch( rumUniqueID i_uiGameID )
{
  switch( OBJECT_ASSET_TYPE( i_uiGameID ) )
  {
    case Creature_AssetType:
    case Portal_AssetType:
    case Widget_AssetType:     return rumPawn::Fetch( i_uiGameID );
    case Broadcast_AssetType:  return rumBroadcast::Fetch( i_uiGameID );
    case Map_AssetType:        return rumMap::Fetch( i_uiGameID );
    case Graphic_AssetType:    return rumGraphic::Fetch( i_uiGameID );
    case Sound_AssetType:      return rumSound::Fetch( i_uiGameID );
    case Inventory_AssetType:  return rumInventory::Fetch( i_uiGameID );
    case Custom_AssetType:     return rumCustom::Fetch( i_uiGameID );
    default:
      rumAssertMsg( false, "Unexpected asset type for game object." );
  }

  return nullptr;
}


// static
Sqrat::Object rumGameObject::FetchVM( rumUniqueID i_uiGameID )
{
  rumGameObject* pcObject{ Fetch( i_uiGameID ) };
  return pcObject ? pcObject->GetScriptInstance() : Sqrat::Object();
}


// virtual
void rumGameObject::Free()
{
  FreeInternal();
}


void rumGameObject::FreeInternal()
{
#if MEMORY_DEBUG
  std::string strInfo{ "Freeing " };
  strInfo += GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( GetGameID() );
  strInfo += "]";
  Logger::LogStandard( strInfo );
#endif // MEMORY_DEBUG

  // Optional script callback
  Sqrat::Object sqObject{ GetScriptInstance() };
  if( sqObject.GetType() == OT_INSTANCE )
  {
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnFree" );
  }

  m_hashProperties.clear();

  // Release the weak reference
  ScriptInstanceRelease();

  if( GetGameID() != INVALID_GAME_ID )
  {
    // Unmanage the object - ideally, this is the only thing keeping the object allocated
    UnmanageScriptObject( GetGameID() );
    if( sqObject.GetType() == OT_INSTANCE )
    {
      const uint32_t uiRefs{ rumScript::GetObjectRefCount( sqObject ) };
      rumAssert( uiRefs <= 1 );
      if( uiRefs > 1 )
      {
        std::string strWarn{ "Warning: Script object " };
        strWarn += GetName();
        strWarn += " [";
        strWarn += rumStringUtils::ToHexString64( GetGameID() );
        strWarn += " ended native control with ";
        strWarn += rumStringUtils::ToString( uiRefs );
        strWarn += " references!";
        Logger::LogStandard( strWarn, Logger::LOG_WARNING );
      }
    }
  }
}


rumAssetID rumGameObject::GetAssetID() const
{
  return( m_pcAsset ? m_pcAsset->GetAssetID() : (rumAssetID)RUM_INVALID_NATIVEHANDLE );
}


const std::string& rumGameObject::GetFilename() const
{
  return( m_pcAsset ? m_pcAsset->GetFilename() : rumStringUtils::NullString() );
}


const std::string& rumGameObject::GetName() const
{
  return( m_pcAsset ? m_pcAsset->GetName() : rumStringUtils::NullString() );
}


Sqrat::Object rumGameObject::GetProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqDefaultValue )
{
  const auto& iter{ m_hashProperties.find( i_ePropertyID ) };
  if( iter == m_hashProperties.end() )
  {
    // Not found in instance properties, so try properties on the asset if there is a pointer to one.
    // NOTE: Player pawns do not set an asset pointer.
    return m_pcAsset ? m_pcAsset->GetProperty( i_ePropertyID, i_sqDefaultValue ) : i_sqDefaultValue;
  }

  return iter->second;
}


Sqrat::Object rumGameObject::GetScriptInstance()
{
  static Sqrat::Object sqNull{ Sqrat::Object() };

  if( m_sqInstance.GetType() == OT_INSTANCE )
  {
    return m_sqInstance;
  }
  else if( m_sqInstance.GetType() == OT_WEAKREF )
  {
    return rumScript::GetWeakReferenceValue( m_sqInstance );
  }

  return sqNull;
}


HSQOBJECT rumGameObject::GetScriptInstanceHandle() const
{
  return m_sqInstance.GetObjectA();
}


bool rumGameObject::HasProperty( rumAssetID i_ePropertyID )
{
  const auto& iter{ m_hashProperties.find( i_ePropertyID ) };
  if( iter == m_hashProperties.end() )
  {
    // Not found in instance properties, so try properties on the asset if there is a pointer to one.
    // NOTE: Player pawns do not set an asset pointer.
    return m_pcAsset ? m_pcAsset->HasProperty( i_ePropertyID ) : false;
  }

  return true;
}


bool rumGameObject::IsServerOnly() const
{
  return m_pcAsset ? m_pcAsset->IsServerOnly() : false;
}


// virtual
void rumGameObject::Manage()
{
  // If this assert fails, the object wasn't added to any list
  rumAssert( false );
}


// static
bool rumGameObject::ManageScriptObject( Sqrat::Object i_sqObject )
{
  if( i_sqObject.GetType() != OT_INSTANCE )
  {
    // Must be an instance
    return false;
  }

  const rumGameObject* pcObject{ i_sqObject.Cast<rumGameObject*>() };
  if( !pcObject )
  {
    // Must inherit from rumGameObject
    return false;
  }

  const rumUniqueID uiGameID{ pcObject->GetGameID() };
  rumAssert( uiGameID != INVALID_GAME_ID );
  if( INVALID_GAME_ID == uiGameID )
  {
    // Must have a valid (non-zero) game id
    return false;
  }

  RUM_COUT_IFDEF( MEMORY_DEBUG,
                  "Managing script object " << pcObject->GetName() << " [" <<
                  rumStringUtils::ToHexString64( uiGameID ) << "]\n" );

  bool bManaged{ false };

  const auto cPair{ s_hashScriptObjects.insert( std::make_pair( uiGameID, i_sqObject ) ) };
  if( cPair.second )
  {
    bManaged = true;
  }
  else
  {
    std::string strError{ "Error: Object " };
    strError += pcObject->GetName();
    strError += " [";
    strError += rumStringUtils::ToHexString64( uiGameID );
    strError += "] already managed";

    Logger::LogStandard( strError );
    rumAssert( cPair.second );
  }

  return bManaged;
}


// virtual
void rumGameObject::OnCreated()
{
  // Add the object to its respective look-up hash
  Manage();

  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnCreated" );
  }
}


// override
void rumGameObject::OnPropertyRemoved( rumAssetID i_ePropertyID )
{
  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnPropertyRemoved", i_ePropertyID );
  }
}


// override
void rumGameObject::OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded )
{
  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnPropertyUpdated", i_ePropertyID, i_sqValue );
  }
}


// static
void rumGameObject::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumFetchObject", FetchVM )
    .Overload<Sqrat::Object( * )( rumAssetID )>( "rumCreate", Create )
    .Overload<Sqrat::Object( * )( rumAssetID, Sqrat::Object )>( "rumCreate", Create )
    .Overload<Sqrat::Object( * )( rumAssetID, Sqrat::Object, Sqrat::Object )>( "rumCreate", Create )
    .Overload<Sqrat::Object( * )( rumAssetID, Sqrat::Object, Sqrat::Object, Sqrat::Object )>( "rumCreate", Create )
    .Overload<Sqrat::Object( * )( rumAssetID, Sqrat::Object, Sqrat::Object, Sqrat::Object, Sqrat::Object )>( "rumCreate", Create )
    .Overload<Sqrat::Object( * )( rumAssetID, Sqrat::Object, Sqrat::Object, Sqrat::Object, Sqrat::Object, Sqrat::Object )>( "rumCreate", Create );

  Sqrat::DerivedClass<rumGameObject, rumPropertyContainer, Sqrat::NoConstructor<rumGameObject>> cGameObject( pcVM, "rumGameObject" );
  cGameObject
    .Func( "GetAssetID", &GetAssetID )
    .Func( "GetOriginType", &GetCreationType )
    .Func( "GetName", &GetName )
    .Func( "GetID", &GetGameID );
  Sqrat::RootTable( pcVM ).Bind( "rumGameObject", cGameObject );
}


Sqrat::Object rumGameObject::ScriptInstanceRelease()
{
  // Temporarily hold onto a ref because the object might be garbage collected right away if it's not a weakref
  Sqrat::Object sqInstance{ m_sqInstance };
  m_sqInstance.Release();
  return sqInstance;
}


void rumGameObject::ScriptInstanceSet( Sqrat::Object i_sqInstance )
{
  rumAssert( i_sqInstance.GetType() == OT_INSTANCE );

  // This will automatically release the current reference
  m_sqInstance = i_sqInstance;
}


std::string rumGameObject::ToString() const
{
  if( m_hashProperties.empty() )
  {
    return rumStringUtils::NullString();
  }

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  std::string strDesc{ "Instance Properties:\n\n" };

  for( const auto& iter : m_hashProperties )
  {
    const rumAssetID ePropertyID{ iter.first };
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
    rumAssert( pcProperty );
    if( pcProperty )
    {
      strDesc += "  " + pcProperty->GetName() + " = ";

      std::string strValue;
      if( pcProperty->IsAssetRef() )
      {
        strValue = rumStringUtils::ToHexString( iter.second.Cast<rumAssetID>() );
      }
      else
      {
        strValue = rumScript::ToString( iter.second );
      }

      const auto cPair{ rumScript::EvalOptionalFunc( Sqrat::RootTable( pcVM ), "OnPropertyDesc", std::string{},
                                                     pcProperty, iter.second ) };
      if( !cPair.second.empty() )
      {
        strDesc += cPair.second + " (" + strValue + ") ";
      }
      else
      {
        strDesc += strValue + " ";
      }

      strDesc += "<" + rumScript::GetObjectTypeName( iter.second ) + ">\n";
    }
  }

  return strDesc;
}


// virtual
void rumGameObject::Unmanage()
{
  // If this assert fails, the object wasn't removed from any list
  rumAssert( false );
}


// static
Sqrat::Object rumGameObject::UnmanageScriptObject( rumUniqueID i_uiGameID )
{
  RUM_COUT_IFDEF( MEMORY_DEBUG, "Unmanaging script object [" << rumStringUtils::ToHexString64( i_uiGameID ) << "]\n" );

  Sqrat::Object sqObject;

  rumAssert( i_uiGameID != INVALID_GAME_ID );

  const auto& iter{ s_hashScriptObjects.find( i_uiGameID ) };
  if( iter != s_hashScriptObjects.end() )
  {
    sqObject = iter->second;
    s_hashScriptObjects.erase( i_uiGameID );
  }

  return sqObject;
}


// static
Sqrat::Object rumGameObject::UnmanageScriptObject( Sqrat::Object i_sqObject )
{
  if( i_sqObject.GetType() != OT_INSTANCE )
  {
    // Must be an instance
    return Sqrat::Object();
  }

  const rumGameObject* pcObject{ i_sqObject.Cast<rumGameObject*>() };
  if( !pcObject )
  {
    // Must inherit from rumGameObject
    return Sqrat::Object();
  }

  return UnmanageScriptObject( pcObject->GetGameID() );
}
