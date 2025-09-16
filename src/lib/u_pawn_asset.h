#ifndef _U_PAWN_ASSET_H_
#define _U_PAWN_ASSET_H_

#include <u_asset.h>
#include <u_pawn.h>

#define SCRIPT_PAWN_ASSET_NATIVE_CLASS "rumPawnAsset"


// An instance of an pawn definition from the game database. These serve as a source or template for individual pawn
// attributes.

class rumPawnAsset : public rumAsset
{
public:

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

  float GetDrawOrder() const
  {
    return m_fDrawOrder;
  }

  void SetDrawOrder( float i_fDrawOrder )
  {
    m_fDrawOrder = i_fDrawOrder;
  }


  rumAssetID GetGraphicID() const
  {
    return m_eGraphicID;
  }

  void SetGraphicID( rumAssetID i_eGraphicID )
  {
    m_eGraphicID = i_eGraphicID;
  }

  uint32_t GetLightRange() const
  {
    return m_uiLightRange;
  }

  void SetLightRange( uint32_t i_uiLightRange )
  {
    m_uiLightRange = i_uiLightRange;
  }

  uint32_t GetMoveType() const
  {
    return m_uiMoveType;
  }

  void SetMoveType( uint32_t i_uiMoveType )
  {
    m_uiMoveType = i_uiMoveType;
  }

  ServiceType GetServiceType() const
  {
    return m_eServiceType;
  }

  void SetServiceType( ServiceType i_eServiceType )
  {
    m_eServiceType = i_eServiceType;
  }

  bool IsServerOnly() const override
  {
    return ( m_eServiceType == Server_ServiceType );
  }

  static rumPawnAsset* Fetch( rumAssetID i_eAssetID );

  static int32_t Init( const std::string& i_strPath );

  static void Shutdown();

  static void ScriptBind();

  void OnCreated( const std::vector<std::string>& i_rvFields ) override;

protected:

  void ExportCSVFile( std::ofstream& o_rcOutfile ) const override;
  void ExportDBTable( std::string& io_strQuery ) const override;

private:

  // A numeric ID that ties this asset to a graphical representation
  rumAssetID m_eGraphicID{ INVALID_ASSET_ID };

  // The move types that will collide with this pawn
  // NOTE: Not to be confused with rumCollisionType! This is intentionally not an enumeration type because the types
  // are defined by scripts.
  uint32_t m_uiCollisionFlags{ 0 };

  // The move type this pawn uses
  // NOTE: Not to be confused with rumMoveFlags! This is intentionally not an enumeration type because the types are
  // defined by scripts.
  uint32_t m_uiMoveType{ 0 };

  // Whether this type is limited to client or server
  ServiceType m_eServiceType{ Shared_ServiceType };

  // The radius of tiles to light
  uint32_t m_uiLightRange{ 0 };

  // The z-order of the object. The smaller the number (including negative), the closer the object is to the viewer.
  float m_fDrawOrder{ 0.f };

  // Whether or not this tile blocks line of sight
  bool m_bBlocksLOS{ false };

  friend class rumCreatureAsset;
  friend class rumPortalAsset;
  friend class rumWidgetAsset;

  using super = rumAsset;
};

#endif // _U_PAWN_ASSET_H_
