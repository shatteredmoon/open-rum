#pragma once

#include <u_asset.h>
#include <u_enum.h>
#include <u_graphic_attributes.h>

#include <unordered_map>

class GraphicAssetAttributes;

class rumGraphicAsset : public rumFileAsset
{
public:

  rumAnimationType GetAnimType() const
  {
    return m_eAnimType;
  }

  void SetAnimType( rumAnimationType i_eAnimType )
  {
    m_eAnimType = i_eAnimType;
  }

  float GetAnimInterval() const
  {
    return m_fAnimInterval;
  }

  void SetAnimInterval( float i_fAnimInterval )
  {
    m_fAnimInterval = i_fAnimInterval;
  }

  AssetType GetAssetType() const override
  {
    return GetClassRegistryID();
  }

  void GetAttributes( rumAssetAttributes& i_rcAttributes ) const override
  {
    i_rcAttributes.Populate( *this );
  }

  uint32_t GetNumAnimFrames() const
  {
    return m_uiNumFrames;
  }

  void SetNumAnimFrames( uint32_t i_uiNumFrames )
  {
    m_uiNumFrames = i_uiNumFrames;
  }

  uint32_t GetNumAnimStates() const
  {
    return m_uiNumStates;
  }

  void SetNumAnimStates( uint32_t i_uiNumStates )
  {
    m_uiNumStates = i_uiNumStates;
  }

  const std::string_view GetTypeName() const override;
  const std::string_view GetTypeSuffix() const override;

  bool IsClientRendered() const
  {
    return m_bClientRendered;
  }

  void SetClientRendered( bool i_bClientRendered )
  {
    m_bClientRendered = i_bClientRendered;
  }

  static void ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile );
  static void ExportDBTables( ServiceType i_eServiceType );

  static rumGraphicAsset* Fetch( rumAssetID i_eAssetID );
  static rumGraphicAsset* FetchByFilename( const std::string& i_strFilename );

  static const std::string GetAssetClassName();

  typedef std::unordered_map< rumAssetID, rumGraphicAsset* > AssetHash;

  static const AssetHash& GetAssetHash()
  {
    return s_hashAssets;
  }

  static const std::string GetAssetTypeSuffix();

  static AssetType GetClassRegistryID()
  {
    return Graphic_AssetType;
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

  void ExportCSVFile( std::ofstream& o_rcOutfile ) const final;
  void ExportDBTable( std::string& io_strQuery ) const final;

  bool LoadFileData() override;
  void SetData( const rumByte* i_pcData, uint32_t i_uiNumBytes ) override;

  // The type defining how animation is handled
  rumAnimationType m_eAnimType{ StandardLooping_AnimationType };

  // The number of animation frames this image has (vertical frames)
  uint32_t m_uiNumFrames{ 1 };

  // The number of sets or states this image has (horizontal frames)
  uint32_t m_uiNumStates{ 1 };

  // Amount of time in seconds between frames
  float m_fAnimInterval{ 0.1f };

  // This is used to show hidden objects in the editor. When false, this image will not be rendered on a client.
  bool m_bClientRendered{ true };

private:

  static rumAssetID s_eNextFreeID;
  static AssetHash s_hashAssets;

  // An optional path hint. This path will be recursively checked first for files if set.
  static std::string s_strPathHint;

  using super = rumFileAsset;
};


class rumGraphicAssetAttributes : public rumAssetAttributes
{
public:

  rumAnimationType GetAnimType() const
  {
    return m_eAnimType;
  }

  float GetAnimInterval() const
  {
    return m_fAnimInterval;
  }

  uint32_t GetNumAnimFrames() const
  {
    return m_uiNumAnimFrames;
  }

  uint32_t GetNumAnimStates() const
  {
    return m_uiNumAnimStates;
  }

  void Populate( const rumGraphicAsset& i_rcAsset ) override
  {
    m_eAnimType = i_rcAsset.GetAnimType();
    m_fAnimInterval = i_rcAsset.GetAnimInterval();
    m_uiNumAnimFrames = i_rcAsset.GetNumAnimFrames();
    m_uiNumAnimStates = i_rcAsset.GetNumAnimStates();
  }

private:

  rumAnimationType m_eAnimType{ StandardLooping_AnimationType };
  uint32_t m_uiNumAnimFrames{ 1 };
  uint32_t m_uiNumAnimStates{ 1 };
  float m_fAnimInterval{ 0.1f };
};
