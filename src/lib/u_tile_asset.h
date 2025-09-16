#ifndef _U_TILE_ASSET_H_
#define _U_TILE_ASSET_H_

#include <u_asset.h>

#include <unordered_map>


// An instance of an tile definition from the game database. These serve as a source or template for individual tile
// settings. Unlike some other game objects, tiles are never instanced, so there is no matching rumTile class.

class rumTileAsset : public rumAsset
{
public:

  AssetType GetAssetType() const override
  {
    return GetClassRegistryID();
  }

  bool GetBlocksLOS() const
  {
    return m_bBlocksLOS;
  }

  void SetBlocksLOS( bool i_bBlocksLOS )
  {
    m_bBlocksLOS = i_bBlocksLOS;
  }

  uint32_t GetCollisionFlags() const
  {
    return m_uiCollisionFlags;
  }

  void SetCollisionFlags( uint32_t i_uiCollisionFlags )
  {
    m_uiCollisionFlags = i_uiCollisionFlags;
  }

  rumAssetID GetGraphicID() const
  {
    return m_eGraphicID;
  }

  void SetGraphicID( rumAssetID i_eGraphicID )
  {
    m_eGraphicID = i_eGraphicID;
  }

  const std::string_view GetTypeName() const override;
  const std::string_view GetTypeSuffix() const override;

  float GetWeight() const
  {
    return m_fWeight;
  }

  void SetWeight( float i_fWeight )
  {
    m_fWeight = i_fWeight;
  }

  bool IsCollision( uint32_t i_uiMoveFlags ) const
  {
    return ( m_uiCollisionFlags & i_uiMoveFlags ) != 0;
  }

  static void ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile );
  static void ExportDBTables( ServiceType i_eServiceType );

  static rumTileAsset* Fetch( rumAssetID i_eAssetID );

  static const std::string GetAssetClassName();

  typedef std::unordered_map< rumAssetID, rumTileAsset* > AssetHash;

  static const AssetHash& GetAssetHash()
  {
    return s_hashAssets;
  }

  static const std::string GetAssetTypeSuffix();

  static AssetType GetClassRegistryID()
  {
    return Tile_AssetType;
  }

  static const std::string GetNativeClassName();

  static rumAssetID GetNextFreeID()
  {
    return s_eNextFreeID;
  }

  static uint32_t GetPixelHeight()
  {
    return m_sPixelHeight;
  }

  static void SetPixelHeight( uint32_t i_uiHeight )
  {
    m_sPixelHeight = i_uiHeight;
  }

  static uint32_t GetPixelWidth()
  {
    return m_sPixelWidth;
  }

  static void SetPixelWidth( uint32_t i_uiWidth )
  {
    m_sPixelWidth = i_uiWidth;
  }

  static const std::string GetPropertyTableCreateQuery();
  static const std::string GetPropertyTableSelectQuery();

  static std::string_view GetStorageName();

  static const std::string GetTableCreateQuery();
  static const std::string GetTableSelectQuery();

  static int32_t Init( const std::string& i_strPath );

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

  static uint32_t m_scDefaultPixelSize;
  static uint32_t m_sPixelHeight;
  static uint32_t m_sPixelWidth;

  static rumAssetID s_eNextFreeID;
  static AssetHash s_hashAssets;

  // A numeric ID that ties this asset to a graphical representation
  rumAssetID m_eGraphicID{ INVALID_ASSET_ID };

  // The movement cost of negotiating this tile
  float m_fWeight{ 0.f };

  // A bitfield of collision info
  uint32_t m_uiCollisionFlags{ 0 };

  // Whether or not this tile blocks line of sight
  bool m_bBlocksLOS{ false };

  using super = rumAsset;
};

#endif // _U_TILE_ASSET_H_
