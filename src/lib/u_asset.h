#pragma once

#include <u_db.h>
#include <u_enum.h>
#include <u_property_container.h>
#include <u_utility.h>

#define ASSET_ID_SUFFIX "ID"

class rumAssetAttributes;
class rumGraphicAsset;
class rumGraphicAssetAttributes;
class rumSoundAsset;
class rumSoundAssetAttributes;


class rumAsset : public rumPropertyContainer
{
public:

  virtual ~rumAsset();

  rumAssetID GetAssetID() const
  {
    return m_eAssetID;
  }

  virtual AssetType GetAssetType() const = 0;

  virtual void GetAttributes( rumAssetAttributes& i_rcAttributes ) const
  {}

  const std::string& GetBaseClassOverride() const
  {
    return m_strBaseClassOverride;
  }

  void SetBaseClassOverride( const std::string& i_strBaseClass )
  {
    // TODO - rebind this asset to scripts
    m_strBaseClassOverride = i_strBaseClass;
  }

  virtual void* GetData() const
  {
    return nullptr;
  }

  virtual uint32_t GetDataAllocSize() const
  {
    return 0;
  }

  virtual const std::string& GetFilename() const
  {
    return rumStringUtils::NullString();
  }

  const std::string& GetName() const
  {
    return m_strName;
  }

  void SetName( const std::string& i_strName )
  {
    // TODO - rebind this asset to scripts
    m_strName = i_strName;
  }

  Sqrat::Object GetScriptClass() const
  {
    return rumScript::GetClass( m_eAssetID );
  }

  virtual const std::string_view GetTypeName() const = 0;
  virtual const std::string_view GetTypeSuffix() const = 0;

  virtual bool IsServerOnly() const
  {
    return false;
  }

  virtual std::string ToString() const;

  template< typename T >
  static T* CreateAsset( const rumAssetID i_eAssetID,
                         const std::string& i_strName,
                         const std::string& i_strBaseClass,
                         const rumStringUtils::StringVector& i_rvFields = {} );

  template< typename T >
  static void ExportCSVFiles( const std::string& i_strPath );

  template< typename T >
  static bool ExportDBTables( ServiceType i_eServiceType );

  static rumAsset* Fetch( rumAssetID i_eAssetID );
  static rumAsset* FetchByName( const std::string& i_strName );

  static rumAssetID GetAssetIDFromConstTable( const std::string& i_strPath );

  // Loads asset from file or db and prepares all assets for script binding and creation
  static int32_t Init( const std::string& i_strPath );

  // Call after scripts have been executed to register an individual script instance class
  template< typename T >
  static void RegisterClass( const rumAsset* i_pcAsset );

  // Call after scripts have been executed to register all script instance classes
  static void RegisterClasses();

  static void ScriptBind();
  static void Shutdown();

protected:

  virtual void ExportCSVFile( std::ofstream& o_rcOutfile ) const;
  void ExportCSVPropertyFile( std::ofstream& o_rcOutfile ) const;

  virtual void ExportDBTable( std::string& io_strQuery ) const;
  void ExportDBPropertyTable( std::string& io_strQuery, const std::string& i_strStorageName,
                              ServiceType i_eServiceType ) const;

  virtual void OnCreated( const rumStringUtils::StringVector& i_rvFields ) = 0;

  void OnPropertyRemoved( rumAssetID i_ePropertyID ) override
  {}

  void OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded ) override
  {}

  static void BindAssetID( rumAssetID i_eAssetID, const std::string& i_strName, const std::string& i_strTypeSuffix );

  template< typename T >
  static T* CreateAsset( const rumStringUtils::StringVector& i_rvFields );

  template< typename T >
  static void LoadAssets( const std::string& i_strPath );

private:

  template< typename T >
  static void ParsePropertyFields( rumStringUtils::StringVector&& i_rcFields );

  void ParseProperty( const std::string& i_strID, const std::string& i_strValue );

  template< typename T >
  static void RegisterClasses();

protected:

  // A numeric ID that represents the asset
  rumAssetID m_eAssetID{ INVALID_ASSET_ID };

  // The asset name
  std::string m_strName;

  // An optional override for the parent class
  std::string m_strBaseClassOverride;
};


class rumFileAsset : public rumAsset
{
public:

  void* GetData() const override
  {
    return m_pcData;
  }

  uint32_t GetDataAllocSize() const override
  {
    return m_uiAllocationSize;
  }

  const std::string& GetFilename() const override
  {
    return m_strFilename;
  }

  void SetFilename( const std::string& i_strFilename )
  {
    m_strFilename = i_strFilename;
  }

  virtual bool LoadFileData() = 0;

  virtual void SetData( const rumByte* i_pcData, uint32_t i_uiNumBytes );

  static void ScriptBind();

protected:

  void ExportCSVFile( std::ofstream& o_rcOutfile ) const override;
  void ExportDBTable( std::string& io_strQuery ) const override;

  // Filename (should not contain any path information)
  std::string m_strFilename;

  // The heap allocated sound data loaded from file
  void* m_pcData{ nullptr };

  // The number of bytes allocated by the data
  uint32_t m_uiAllocationSize{ 0 };

private:

  using super = rumAsset;
};


// TODO - turn these into engine properties?
class rumAssetAttributes
{
public:

  virtual void Populate( const rumAsset& i_rcAsset )
  {}

  virtual void Populate( const rumGraphicAsset& i_rcAsset )
  {}

  virtual void Populate( const rumSoundAsset& i_rcAsset )
  {}
};

#include <u_asset.inl>
