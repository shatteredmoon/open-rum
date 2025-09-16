#ifndef _U_INVENTORY_ASSET_H_
#define _U_INVENTORY_ASSET_H_

#include <u_asset.h>

#include <unordered_map>


// An instance of an inventory definition from the game database. Unlike the Inventory class, these are not in-game
// objects, but instead serve as source or template for individual inventory settings. Instances of inventory objects
// in game are instantiated with the rumInventory class.

class rumInventoryAsset : public rumAsset
{
public:

  AssetType GetAssetType() const override
  {
    return GetClassRegistryID();
  }

  ClientReplicationType GetClientReplicationType() const
  {
    return m_eClientReplicationType;
  }

  void SetClientReplicationType( ClientReplicationType i_eReplicationType )
  {
    m_eClientReplicationType = i_eReplicationType;
  }

  const std::string_view GetTypeName() const override;
  const std::string_view GetTypeSuffix() const override;

  bool IsGlobal() const
  {
    return( Global_ClientReplicationType == m_eClientReplicationType );
  }

  bool IsRegional() const
  {
    return( Regional_ClientReplicationType == m_eClientReplicationType );
  }

  bool IsPrivate() const
  {
    return( Private_ClientReplicationType == m_eClientReplicationType );
  }

  bool IsPersistent() const
  {
    return m_bPersistent;
  }

  void SetPersistence( bool i_bPersistent )
  {
    m_bPersistent = i_bPersistent;
  }

  static void ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile );
  static void ExportDBTables( ServiceType i_eServiceType );

  static rumInventoryAsset* Fetch( rumAssetID i_eAssetID );

  static const std::string GetAssetClassName();

  typedef std::unordered_map< rumAssetID, rumInventoryAsset* > AssetHash;

  static const AssetHash& GetAssetHash()
  {
    return s_hashAssets;
  }

  static const std::string GetAssetTypeSuffix();

  static AssetType GetClassRegistryID()
  {
    return Inventory_AssetType;
  }

  static const std::string GetNativeClassName();

  static rumAssetID GetNextFreeID()
  {
    return s_eNextFreeID;
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

  static rumAssetID s_eNextFreeID;
  static AssetHash s_hashAssets;

  // How property changes are replicated to clients
  ClientReplicationType m_eClientReplicationType{ None_ClientReplicationType };

  // Are server-side property changes saved to db?
  bool m_bPersistent{ false };

  using super = rumAsset;
};

#endif // _U_INVENTORY_ASSET_H_
