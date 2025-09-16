#pragma once

#include <u_property_container.h>
#include <u_structs.h>
#include <u_utility.h>

#include <unordered_map>

class DBresult;
class rumAsset;
class rumResource;


// Any object that is tangible to gameplay. All pawns, maps, sound, graphics, etc. extend from this class.
class rumGameObject : public rumPropertyContainer
{
public:

  virtual ~rumGameObject();

  // Called when the object should no longer be used. This releases the underlying script instance if one is held and
  // will also free data held in member variables. Do not attempt to use this object again after calling Free.
  virtual void Free();

  const rumAsset* GetAsset() const
  {
    return m_pcAsset;
  }

  rumAssetID GetAssetID() const;

  const std::string& GetFilename() const;
  const std::string& GetName() const;

  // Returns the ID of the object that is shared between server and client. The client typically receives the UID from
  // the server, but there may be times when an object will exist client-side only.
  rumUniqueID GetGameID() const
  {
    return m_uiGameID;
  }

  virtual void OnCreated();

  // For determining if the object was created on the server or the client
  ObjectCreationType GetCreationType() const
  {
    return OBJECT_CREATION_TYPE( m_uiGameID );
  }

  Sqrat::Object ScriptGetUniversalID() const;

  Sqrat::Object GetScriptInstance();
  void ScriptInstanceSet( Sqrat::Object i_sqInstance );

  HSQOBJECT GetScriptInstanceHandle() const;

  Sqrat::Object GetProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqDefaultValue ) override;

  // TODO - Why does this have to be here? It is already a public non-virtual in the base class.
  Sqrat::Object GetProperty(rumAssetID i_ePropertyID)
  {
    return GetProperty( i_ePropertyID, Sqrat::Object() );
  }

  bool HasProperty( rumAssetID i_ePropertyID ) override;

  bool IsServerOnly() const;

  virtual void Tick()
  {}

  std::string ToString() const override;

  // Method for creating a raw object by class name without an associated asset ID
  static Sqrat::Object Create( const Sqrat::Object& i_sqClass, Sqrat::Table i_sqParams, rumUniqueID i_uiGameID );

  static Sqrat::Object Create( const Sqrat::Object& i_sqClass, Sqrat::Table i_sqParams )
  {
    return Create( i_sqClass, i_sqParams, INVALID_GAME_ID );
  }

  static Sqrat::Object Create( const Sqrat::Object& i_sqClass )
  {
    return Create( i_sqClass, Sqrat::Table(), INVALID_GAME_ID );
  }

  // Factory for creating any asset by ID
  static Sqrat::Object Create( rumAssetID i_eAssetID, Sqrat::Table i_sqParams, rumUniqueID i_uiGameID );

  static Sqrat::Object Create( rumAssetID i_eAssetID )
  {
    return Create( i_eAssetID, Sqrat::Table(), INVALID_GAME_ID );
  }

  static Sqrat::Object Create( rumAssetID i_eAssetID, Sqrat::Object i_sqParam1 )
  {
    Sqrat::Table sqParams( Sqrat::DefaultVM::Get() );
    sqParams.SetValue( (SQInteger)0, i_sqParam1 );
    return Create( i_eAssetID, sqParams, INVALID_GAME_ID );
  }

  static Sqrat::Object Create( rumAssetID i_eAssetID, Sqrat::Object i_sqParam1, Sqrat::Object i_sqParam2 )
  {
    Sqrat::Table sqParams( Sqrat::DefaultVM::Get() );
    sqParams.SetValue( (SQInteger)0, i_sqParam1 );
    sqParams.SetValue( (SQInteger)1, i_sqParam2 );
    return Create( i_eAssetID, sqParams, INVALID_GAME_ID );
  }

  static Sqrat::Object Create( rumAssetID i_eAssetID, Sqrat::Object i_sqParam1, Sqrat::Object i_sqParam2,
                               Sqrat::Object i_sqParam3 )
  {
    Sqrat::Table sqParams( Sqrat::DefaultVM::Get() );
    sqParams.SetValue( (SQInteger)0, i_sqParam1 );
    sqParams.SetValue( (SQInteger)1, i_sqParam2 );
    sqParams.SetValue( (SQInteger)2, i_sqParam3 );
    return Create( i_eAssetID, sqParams, INVALID_GAME_ID );
  }

  static Sqrat::Object Create( rumAssetID i_eAssetID, Sqrat::Object i_sqParam1, Sqrat::Object i_sqParam2,
                               Sqrat::Object i_sqParam3, Sqrat::Object i_sqParam4 )
  {
    Sqrat::Table sqParams( Sqrat::DefaultVM::Get() );
    sqParams.SetValue( (SQInteger)0, i_sqParam1 );
    sqParams.SetValue( (SQInteger)1, i_sqParam2 );
    sqParams.SetValue( (SQInteger)2, i_sqParam3 );
    sqParams.SetValue( (SQInteger)3, i_sqParam4 );
    return Create( i_eAssetID, sqParams, INVALID_GAME_ID );
  }

  static Sqrat::Object Create( rumAssetID i_eAssetID, Sqrat::Object i_sqParam1, Sqrat::Object i_sqParam2,
                               Sqrat::Object i_sqParam3, Sqrat::Object i_sqParam4, Sqrat::Object i_sqParam5 )
  {
    Sqrat::Table sqParams( Sqrat::DefaultVM::Get() );
    sqParams.SetValue( (SQInteger)0, i_sqParam1 );
    sqParams.SetValue( (SQInteger)1, i_sqParam2 );
    sqParams.SetValue( (SQInteger)2, i_sqParam3 );
    sqParams.SetValue( (SQInteger)3, i_sqParam4 );
    sqParams.SetValue( (SQInteger)4, i_sqParam5 );
    return Create( i_eAssetID, sqParams, INVALID_GAME_ID );
  }

  // Request from scripts to reference a script object so that it isn't garbage collected
  static bool ManageScriptObject( Sqrat::Object i_sqObject );
  static Sqrat::Object UnmanageScriptObject( rumUniqueID i_uiGameID );
  static Sqrat::Object UnmanageScriptObject( Sqrat::Object i_sqObject );

  static void Shutdown()
  {}

  static rumGameObject* Fetch( rumUniqueID i_uiGameID );
  static Sqrat::Object FetchVM( rumUniqueID i_uiGameID );

  static void ScriptBind();

protected:

  virtual void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID )
  {
    // TODO - Get rid of this when everything has a proper override
    rumAssert( false );
  }

  virtual Sqrat::Object ScriptInstanceRelease();

  void SetGameID( rumUniqueID i_uiGameID )
  {
    m_uiGameID = i_uiGameID;
  }

protected:

  virtual void Manage();
  virtual void Unmanage();

  void OnPropertyRemoved( rumAssetID i_ePropertyID ) override;
  void OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded ) override;

private:

  void FreeInternal();

protected:

  rumAsset* m_pcAsset{ nullptr };

private:

  rumUniqueID m_uiGameID{ INVALID_GAME_ID };

  Sqrat::Object m_sqInstance;

  // Managed script objects. This container is used by scripts to request native management of script objects,
  // essentially adding a ref to the script object so that it's not garbage collected automatically when the object's
  // number of references falls to zero. It is up to scripts to add and remove objects from this container.
  using ScriptObjectContainer = std::unordered_map<rumUniqueID, Sqrat::Object>;
  static ScriptObjectContainer s_hashScriptObjects;

  using super = rumPropertyContainer;
};
