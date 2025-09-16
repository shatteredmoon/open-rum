#pragma once

#include <u_asset.h>

#include <unordered_map>


// Supported sound types on the client are WAV, AIFF, MP3, MP2, MP1, OGG, and MIDI
enum rumSoundDataType
{
  Invalid_SoundDataType = 0,
  WAV_SoundDataType,
  AIFF_SoundDataType,
  MP1_SoundDataType,
  MP2_SoundDataType,
  MP3_SoundDataType,
  OGG_SoundDataType,
  MIDI_SoundDataType
};


class rumSoundAsset : public rumFileAsset
{
public:

  AssetType GetAssetType() const override
  {
    return GetClassRegistryID();
  }

  void GetAttributes( rumAssetAttributes& i_rcAttributes ) const override
  {
    i_rcAttributes.Populate( *this );
  }

  rumSoundDataType GetDataType() const
  {
    return m_eDataType;
  }

  const std::string_view GetTypeName() const override;
  const std::string_view GetTypeSuffix() const override;

  static void ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile );
  static void ExportDBTables( ServiceType i_eServiceType );

  static rumSoundAsset* Fetch( rumAssetID i_eAssetID );
  static rumSoundAsset* FetchByName( const std::string& i_strName );
  static rumSoundAsset* FetchByFilename( const std::string& i_strFilename );

  static const std::string GetAssetClassName();

  typedef std::unordered_map< rumAssetID, rumSoundAsset* > AssetHash;

  static const AssetHash& GetAssetHash()
  {
    return s_hashAssets;
  }

  static const std::string GetAssetTypeSuffix();

  static AssetType GetClassRegistryID()
  {
    return Sound_AssetType;
  }

  static const std::string GetNativeClassName();

  static rumAssetID GetNextFreeID()
  {
    return s_eNextFreeID;
  }

  static std::string& GetPathHint()
  {
    return s_strPathHint;
  }

  static void SetPathHint( const std::string& i_strPath )
  {
    s_strPathHint = i_strPath;
  }

  static const std::string GetPropertyTableCreateQuery();
  static const std::string GetPropertyTableSelectQuery();

  static std::string_view GetStorageName();

  static const std::string GetTableCreateQuery();
  static const std::string GetTableSelectQuery();

  static int32_t Init( const std::string& i_strPath );

  static bool LoadArchive( const std::string& i_strArchive );
  static bool LoadData( rumAssetID i_eAssetID );

  static void RefreshFileData();

  static void Remove( rumAssetID i_eAssetID )
  {
    s_hashAssets.erase( i_eAssetID );
  }

  static void ScriptBind();

  static void Shutdown();

  void OnCreated( const std::vector<std::string>& i_rvFields ) override;

private:

  void DetermineSoundDataType();
  void ExportCSVFile( std::ofstream& o_rcOutfile ) const final;
  void ExportDBTable( std::string& io_strQuery ) const final;

  bool LoadFileData() override;
  void SetData( const rumByte* i_pcData, uint32_t i_uiNumBytes ) override;

protected:

  rumSoundDataType m_eDataType{ Invalid_SoundDataType };

private:

  static rumAssetID s_eNextFreeID;
  static AssetHash s_hashAssets;

  // An optional path hint. This path will be recursively checked first for files if set.
  static std::string s_strPathHint;

  using super = rumFileAsset;
};


class rumSoundAssetAttributes : public rumAssetAttributes
{
public:

  rumSoundDataType GetDataType() const
  {
    return m_eDataType;
  }

  void Populate( const rumSoundAsset& i_rcAsset ) override
  {
    m_eDataType = i_rcAsset.GetDataType();
  }

private:

  rumSoundDataType m_eDataType{ Invalid_SoundDataType };
};
