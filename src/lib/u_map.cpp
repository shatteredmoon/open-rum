#include <u_map.h>

#include <u_assert.h>
#include <u_db.h>
#include <u_log.h>
#include <u_map_asset.h>
#include <u_pawn.h>
#include <u_pawn_asset.h>
#include <u_player.h>
#include <u_pos_iterator.h>
#include <u_property_asset.h>
#include <u_resource.h>
#include <u_tile_asset.h>
#include <u_zlib.h>

#include <vector>

// Static initializations
Sqrat::Object rumMap::s_sqClass;
rumMap::MapHash rumMap::s_hashMaps;


rumMap::~rumMap()
{
  RUM_COUT( "Freeing map " << GetName() << " [" << rumStringUtils::ToHexString( GetAssetID() ) << "]\n" );

  FreeInternal();

  rumAssertMsg( m_hashPawns.size() == 0, "Map still has pawns" );
  rumAssertMsg( m_hashPlayers.size() == 0, "Map still has players" );
  rumAssertMsg( nullptr == m_pcData, "Map still has map data" );
  rumAssertMsg( 0 == m_uiRows, "Map rows != 0" );
  rumAssertMsg( 0 == m_uiCols, "Map columns != 0" );
}


rumMap::PositionData* rumMap::AccessPositionData( const rumPosition& i_rcPos ) const
{
  rumPosition posValidated( i_rcPos );
  if( ValidatePosition( posValidated ) == rumPosition::POSITION_OK )
  {
    return &( m_pcData[posValidated.m_iY][posValidated.m_iX] );
  }

  return nullptr;
}


bool rumMap::AddPawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    rumAssert( false );
    return false;
  }

  rumPosition cNewPos( i_rcPos );
  if( ValidatePosition( cNewPos ) != rumPosition::POSITION_OK )
  {
    return false;
  }

  const auto p( m_hashPawns.insert( i_pcPawn->GetGameID() ) );
  if( !p.second )
  {
    rumAssert( false );
    return false;
  }

  // Set the map ID and initial position
  i_pcPawn->SetMapID( GetGameID(), cNewPos );

  PositionData* pcPosition{ AccessPositionData( cNewPos ) };
  if( !pcPosition || !pcPosition->InsertPawn( i_pcPawn ) )
  {
    rumAssertMsg( false, "Error: Failed to insert the pawn at it's new map position" );
  }

  if( i_pcPawn->IsPlayer() )
  {
    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( i_pcPawn->GetPlayerID() ) };
    if( pcPlayer && AddPlayer( pcPlayer ) != RESULT_SUCCESS )
    {
      rumAssertMsg( false, "Error: Failed to add player data to the map!" );
    }
  }

  return true;
}


bool rumMap::AddPawn( rumUniqueID i_uiPawnID, const rumPosition& i_rcPos )
{
  rumPawn* pcPawn{ rumPawn::Fetch( i_uiPawnID ) };
  if( pcPawn )
  {
    return AddPawn( pcPawn, i_rcPos );
  }

  return false;
}


bool rumMap::AddPawnVM( Sqrat::Object i_sqObject, const rumPosition& i_rcPos )
{
  bool bAdded{ false };

  if( i_sqObject.GetType() == OT_INSTANCE )
  {
    rumPawn* pcPawn{ i_sqObject.Cast<rumPawn*>() };
    if( pcPawn )
    {
      bAdded = AddPawn( pcPawn, i_rcPos );
      if( bAdded )
      {
        // Hold a reference to this script object
        ManageScriptObject( i_sqObject );
      }
    }
  }

  return bAdded;
}


// virtual
int32_t rumMap::AddPlayer( rumPlayer* i_pcPlayer )
{
  if( !i_pcPlayer )
  {
    return RESULT_FAILED;
  }

  int32_t eResult{ RESULT_FAILED };

  std::string strInfo{ "Adding player " };
  strInfo += i_pcPlayer->GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( i_pcPlayer->GetPlayerID() );
  strInfo += "] to Map ";
  strInfo += GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( GetGameID() );
  strInfo += "]";
  Logger::LogStandard( strInfo );

  // For now, we are just adding the player's socket
  const auto p( m_hashPlayers.insert( i_pcPlayer->GetPlayerID() ) );
  if( p.second )
  {
    eResult = RESULT_SUCCESS;
  }

  RUM_COUT( "Map has " << m_hashPlayers.size() << " player(s)" << '\n' );

  rumPawn* pcPawn{ i_pcPlayer->GetPlayerPawn() };
  if( pcPawn )
  {
    // Notify scripts that the player has been added to a map
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnPlayerAdded", pcPawn->GetScriptInstance() );
  }

  return eResult;
}


void rumMap::AppendWrappedPositions( const rumPosition& i_rcPos, std::vector<rumPosition>& i_rcPositions ) const
{
  if( m_bWraps )
  {
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX - m_uiCols, i_rcPos.m_iY ) );
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX + m_uiCols, i_rcPos.m_iY ) );
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX, i_rcPos.m_iY - m_uiRows ) );
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX, i_rcPos.m_iY + m_uiRows ) );
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX - m_uiCols, i_rcPos.m_iY - m_uiRows ) );
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX - m_uiCols, i_rcPos.m_iY + m_uiRows ) );
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX + m_uiCols, i_rcPos.m_iY - m_uiRows ) );
    i_rcPositions.push_back( rumPosition( i_rcPos.m_iX + m_uiCols, i_rcPos.m_iY + m_uiRows ) );
  }
}


// static
rumMap* rumMap::Fetch( rumUniqueID i_uiGameID )
{
  const auto& iter{ s_hashMaps.find( i_uiGameID ) };
  return iter != s_hashMaps.end() ? iter->second : nullptr;
}


// override
void rumMap::Free()
{
  FreeInternal();
  super::Free();
}


void rumMap::FreeCellData()
{
  // Clear all cells
  for( uint32_t j = 0; j < m_uiRows; ++j )
  {
    for( uint32_t i = 0; i < m_uiCols; ++i )
    {
      m_pcData[j][i].m_cPawnDataList.clear();
    }

    if( m_pcData[j] )
    {
      delete[] m_pcData[j];
      m_pcData[j] = nullptr;
    }
  }

  if( m_pcData )
  {
    delete[] m_pcData;
    m_pcData = nullptr;
  }

  m_uiMapSize = m_uiRows = m_uiCols = 0;
}


void rumMap::FreeInternal()
{
  // Remove all pawns
  for( const auto iter : m_hashPawns )
  {
    const rumUniqueID uiPawnID{ iter };

    rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
    rumAssert( pcPawn );
    if( pcPawn && !pcPawn->IsPlayer() )
    {
      // Player pawns are only released from memory during client exit or player logout
      pcPawn->Free();
    }
  }

  // Remove all local pawns
  m_hashPawns.clear();

  // Remove all local players
  m_hashPlayers.clear();

  FreeCellData();

  Unmanage();
}


rumVector rumMap::GetDirectionVector( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget )
{
  rumVector vDir( 0, 0 );

  if( i_rcTarget.m_iX > i_rcOrigin.m_iX )
  {
    vDir.m_fX = 1;
  }
  else if( i_rcTarget.m_iX < i_rcOrigin.m_iX )
  {
    vDir.m_fX = -1;
  }

  if( i_rcTarget.m_iY > i_rcOrigin.m_iY )
  {
    vDir.m_fY = 1;
  }
  else if( i_rcTarget.m_iY < i_rcOrigin.m_iY )
  {
    vDir.m_fY = -1;
  }

  return vDir;
}


float rumMap::GetDistance( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget ) const
{
  // Early out if target and origin positions are the same
  if( i_rcTarget == i_rcOrigin )
  {
    return 0.f;
  }

  std::vector<rumPosition> vPositions;
  vPositions.push_back( i_rcTarget );

  if( m_bWraps )
  {
    // Add all potential wrapped locations for the player
    AppendWrappedPositions( i_rcTarget, vPositions );
  }

  uint64_t ulNearestDistance{ UINT64_MAX };

  for( uint32_t i = 0; i < vPositions.size(); ++i )
  {
    const rumPosition& rcTestPos{ vPositions[i] };

    // Get the difference between the two positions
    const uint32_t dx{ (uint32_t)( abs( rcTestPos.m_iX - i_rcOrigin.m_iX ) ) };
    const uint32_t dy{ (uint32_t)( abs( rcTestPos.m_iY - i_rcOrigin.m_iY ) ) };
    const uint64_t dt{ SQR( dx ) + SQR( dy ) };

    ulNearestDistance = std::min( dt, ulNearestDistance );
  }

  return (float)sqrt( (long double)ulNearestDistance );
}


rumPlayer* rumMap::GetNearestPlayer( const rumPosition& i_rcPos, uint32_t i_uiMaxTileDistance, rumDirectionType i_eDir,
                                     bool i_bCheckLOS ) const
{
  rumPlayer* pcNearestPlayer{ nullptr };
  uint32_t uiNearestDistance{ UINT32_MAX };

  for( const auto iter : m_hashPlayers )
  {
    const rumUniqueID uiPlayerID{ iter };

    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( uiPlayerID ) };
    if( pcPlayer )
    {
      const rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
      if( pcPawn )
      {
        const rumPosition& rcPlayerPos{ pcPawn->GetPos() };

        std::vector<rumPosition> vPositions;
        vPositions.push_back( rcPlayerPos );

        if( m_bWraps )
        {
          // Add all potential wrapped locations for the player
          AppendWrappedPositions( rcPlayerPos, vPositions );
        }

        for( uint32_t i = 0; i < vPositions.size(); ++i )
        {
          const rumPosition& rcTestPos{ vPositions[i] };

          // Get the difference between the two positions
          const uint32_t dx{ (uint32_t)( abs( rcTestPos.m_iX - i_rcPos.m_iX ) ) };
          const uint32_t dy{ (uint32_t)( abs( rcTestPos.m_iY - i_rcPos.m_iY ) ) };

          uint32_t uiTileDistance = 0;

          if( Intercardinal_DirectionType == i_eDir )
          {
            // Number of steps between the two positions, diagonal movement allowed
            uiTileDistance = std::max( dx, dy );
          }
          else
          {
            // Number of steps between the two positions, diagonal movement NOT allowed
            uiTileDistance = dx + dy;
          }

          // See if this player is closer than the current nearest player
          if( uiTileDistance < uiNearestDistance && uiTileDistance <= i_uiMaxTileDistance )
          {
            bool bVisible{ true };
            if( i_bCheckLOS )
            {
              // Since map wrapping is already accounted for, call the internal LOS func
              bVisible = HasLineOfSightInternal( i_rcPos, rcTestPos );
            }

            if( bVisible )
            {
              uiNearestDistance = uiTileDistance;
              pcNearestPlayer = pcPlayer;
            }
          }
        }
      }
    }
  }

  return pcNearestPlayer;
}


rumPositionIterator rumMap::GetPositionIterator( const rumPosition& i_rcPos ) const
{
  return rumPositionIterator( AccessPositionData( i_rcPos ) );
}


rumPositionIterator rumMap::GetPositionIterator( int32_t i_iX, int32_t i_iY ) const
{
  return GetPositionIterator( rumPosition( i_iX, i_iY ) );
}


uint32_t rumMap::GetTileDistance( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                                  const rumDirectionType i_eDir ) const
{
  // Early out if target and origin positions are the same
  if( i_rcTarget == i_rcOrigin )
  {
    return 0;
  }

  std::vector<rumPosition> vPositions;
  vPositions.push_back( i_rcTarget );

  if( m_bWraps )
  {
    // Add all potential wrapped locations for the player
    AppendWrappedPositions( i_rcTarget, vPositions );
  }

  uint32_t uiNearestDistance{ UINT32_MAX };

  for( uint32_t i = 0; i < vPositions.size(); ++i )
  {
    const rumPosition& rcTestPos{ vPositions[i] };

    // Get the difference between the two positions
    const uint32_t dx{ (uint32_t)abs( rcTestPos.m_iX - i_rcOrigin.m_iX ) };
    const uint32_t dy{ (uint32_t)abs( rcTestPos.m_iY - i_rcOrigin.m_iY ) };

    uint32_t dt{ 0 };

    if( i_eDir == Intercardinal_DirectionType )
    {
      // Number of steps between the two positions, diagonal movement allowed
      dt = std::max( dx, dy );
    }
    else
    {
      // Number of steps between the two positions, diagonal movement NOT allowed
      dt = dx + dy;
    }

    uiNearestDistance = std::min( dt, uiNearestDistance );
  }

  return uiNearestDistance;
}


bool rumMap::HasClearPath( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                           rumMoveFlags i_uiMovementFlags ) const
{
  // Early out if target and origin positions are the same
  if( i_rcOrigin == i_rcTarget )
  {
    return true;
  }

  std::vector<rumPosition> vPositions;
  vPositions.push_back( i_rcTarget );

  if( m_bWraps )
  {
    // Add all potential wrapped locations for the player
    AppendWrappedPositions( i_rcTarget, vPositions );
  }

  bool bClearPath{ false };

  // Visit each position, but discontinue early if there is an obstacle
  for( uint32_t i = 0; !bClearPath && i < vPositions.size(); ++i )
  {
    bClearPath = HasClearPathInternal( i_rcOrigin, vPositions[i], i_uiMovementFlags );
  }

  return bClearPath;
}


bool rumMap::HasClearPathInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                                   rumMoveFlags i_uiMovementFlags ) const
{
  // Early out if target and origin positions are the same
  if( i_rcOrigin == i_rcTarget )
  {
    return true;
  }

  // Some implementation of Bresenham's line drawing algorithm ripped off of the net and slightly modified
  bool bHasClearPath{ true };
  bool bTargetReached{ false };

  int32_t stepx{ 1 };
  int32_t stepy{ 1 };

  int32_t dy{ i_rcTarget.m_iY - i_rcOrigin.m_iY };
  int32_t dx{ i_rcTarget.m_iX - i_rcOrigin.m_iX };

  if( dy < 0 )
  {
    dy = -dy;
    stepy = -1;
  }

  if( dx < 0 )
  {
    dx = -dx;
    stepx = -1;
  }

  dy <<= 1; // dy is now 2*dy
  dx <<= 1; // dx is now 2*dx

  rumPosition cPos( i_rcOrigin );

  if( dx > dy )
  {
    int32_t iFraction{ dy - ( dx >> 1 ) }; // same as 2*dy - dx
    while( cPos.m_iX != i_rcTarget.m_iX && bHasClearPath )
    {
      if( iFraction >= 0 )
      {
        cPos.m_iY += stepy;
        iFraction -= dx; // same as fraction -= 2*dx
      }
      cPos.m_iX += stepx;
      iFraction += dy; // same as fraction -= 2*dy

      bTargetReached = ( cPos == i_rcTarget );
      bHasClearPath = !IsTileCollision( cPos, i_uiMovementFlags );
    }
  }
  else
  {
    int32_t iFraction{ dx - ( dy >> 1 ) };
    while( cPos.m_iY != i_rcTarget.m_iY && bHasClearPath )
    {
      if( iFraction >= 0 )
      {
        cPos.m_iX += stepx;
        iFraction -= dy;
      }
      cPos.m_iY += stepy;
      iFraction += dx;

      bTargetReached = ( cPos == i_rcTarget );
      bHasClearPath = !IsTileCollision( cPos, i_uiMovementFlags );
    }
  }

  return bTargetReached;
}


bool rumMap::HasLineOfSight( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                             uint32_t i_uiMaxTileDistance, rumDirectionType i_eDir ) const
{
  // Shortuct if target and origin positions are the same
  if( i_rcOrigin == i_rcTarget )
  {
    return true;
  }

  std::vector<rumPosition> vPositions;
  vPositions.push_back( i_rcTarget );

  if( m_bWraps )
  {
    // Add all potential wrapped locations for the player
    AppendWrappedPositions( i_rcTarget, vPositions );
  }

  bool bVisible{ false };

  // Visit each position, but discontinue early if the target position is within LOS
  for( uint32_t i = 0; !bVisible && i < vPositions.size(); ++i )
  {
    if( IsWithinTileDistanceInternal( i_rcOrigin, i_rcTarget, i_uiMaxTileDistance, i_eDir ) )
    {
      bVisible = HasLineOfSightInternal( i_rcOrigin, vPositions[i] );
    }
  }

  return bVisible;
}


bool rumMap::HasLineOfSightInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget ) const
{
  // Shortuct if target and origin positions are the same
  if( i_rcOrigin == i_rcTarget )
  {
    return true;
  }

  // Some implementation of Bresenham's line drawing algorithm ripped off of the net and slightly modified
  bool bHasLOS{ true };
  bool bTargetReached{ false };

  int32_t stepx{ 1 };
  int32_t stepy{ 1 };

  int32_t dy{ i_rcTarget.m_iY - i_rcOrigin.m_iY };
  int32_t dx{ i_rcTarget.m_iX - i_rcOrigin.m_iX };

  if( dy < 0 )
  {
    dy = -dy;
    stepy = -1;
  }

  if( dx < 0 )
  {
    dx = -dx;
    stepx = -1;
  }

  dy <<= 1; // dy is now 2*dy
  dx <<= 1; // dx is now 2*dx

  rumPosition cPos( i_rcOrigin );

  if( dx > dy )
  {
    int32_t iFraction{ dy - ( dx >> 1 ) }; // same as 2*dy - dx
    while( cPos.m_iX != i_rcTarget.m_iX && bHasLOS )
    {
      if( iFraction >= 0 )
      {
        cPos.m_iY += stepy;
        iFraction -= dx; // same as fraction -= 2*dx
      }
      cPos.m_iX += stepx;
      iFraction += dy; // same as fraction -= 2*dy

      bTargetReached = ( cPos == i_rcTarget );
      bHasLOS = !PositionBlocksSight( cPos );
    }
  }
  else
  {
    int32_t iFraction{ dx - ( dy >> 1 ) };
    while( cPos.m_iY != i_rcTarget.m_iY && bHasLOS )
    {
      if( iFraction >= 0 )
      {
        cPos.m_iX += stepx;
        iFraction -= dy;
      }
      cPos.m_iY += stepy;
      iFraction += dx;

      bTargetReached = ( cPos == i_rcTarget );
      bHasLOS = !PositionBlocksSight( cPos );
    }
  }

  return bTargetReached;
}


rumCollisionType rumMap::IsCollision( const rumPosition& i_rcPos, rumPawn* i_pcPawn,
                                      rumMoveFlags i_eMovementFlags ) const
{
  rumCollisionType eCollision{ Error_CollisionType };

  rumPosition rPosValidated( i_rcPos );

  const rumPosition::PositionValidationEnum eStatus{ ValidatePosition( rPosValidated ) };
  if( rumPosition::POSITION_OK == eStatus )
  {
    if( !( i_eMovementFlags & IgnoreTileCollision_MoveFlag ) && IsTileCollision( rPosValidated, i_pcPawn ) )
    {
      eCollision = Tile_CollisionType;
    }
    else if( !( i_eMovementFlags & IgnorePawnCollision_MoveFlag ) && IsPawnCollision( rPosValidated, i_pcPawn ) )
    {
      eCollision = Pawn_CollisionType;
    }
    else
    {
      eCollision = None_CollisionType;
    }
  }

  return eCollision;
}


bool rumMap::IsHarmful( const rumPosition& i_rcPos, rumPawn* i_pcPawn ) const
{
  if( !i_pcPawn )
  {
    return false;
  }

  const PositionData* pcPosition{ GetPositionData( i_rcPos ) };
  if( pcPosition )
  {
    // Optional script callback
    const auto cPair{ rumScript::EvalOptionalFunc( i_pcPawn->GetScriptInstance(), "IsPositionHarmful", false,
                                                   i_rcPos ) };
    return cPair.second;
  }

  return false;
}


bool rumMap::IsPawnCollision( const rumPosition& i_rcPos, rumPawn* i_pcPawn ) const
{
  if( !i_pcPawn )
  {
    return false;
  }

  bool bCollision{ false };

  const PositionData* pcPosition{ GetPositionData( i_rcPos ) };
  if( pcPosition )
  {
    HSQUIRRELVM vm{ Sqrat::DefaultVM::Get() };

    auto iter( pcPosition->m_cPawnDataList.begin() );
    const auto end( pcPosition->m_cPawnDataList.end() );
    while( !bCollision && iter != end )
    {
      rumPawn* pcPawnData{ rumPawn::Fetch( iter->m_iPawnID ) };
      if( pcPawnData && pcPawnData->IsVisible() )
      {
        // Optional script callback
        const auto cPair{ rumScript::EvalOptionalFunc( pcPawnData->GetScriptInstance(),
                                                       "OnCollisionTest",
                                                       false /* non-collision */,
                                                       i_pcPawn->GetScriptInstance() ) };
        if( !cPair.first )
        {
          // The optional collision test func doesn't exist, so call the default collision test
          bCollision = ( pcPawnData->IsCollision( i_pcPawn->GetMoveType() ) != None_CollisionType );
        }
        else
        {
          bCollision = cPair.second;
        }
      }

      ++iter;
    }
  }

  return bCollision;
}


bool rumMap::IsTileCollision( const rumPosition& i_rcPos, uint32_t i_uiMoveFlags ) const
{
  bool bCollision{ true };

  const PositionData* pcPosition{ GetPositionData( i_rcPos ) };
  if( pcPosition )
  {
    const rumTileAsset* pcTile{ rumTileAsset::Fetch( pcPosition->m_eTileID ) };
    if( pcTile )
    {
      bCollision = pcTile->IsCollision( i_uiMoveFlags );
    }
  }

  return bCollision;
}


bool rumMap::IsTileCollision( const rumPosition& i_rcPos, rumPawn* i_pcPawn ) const
{
  if( !i_pcPawn )
  {
    return true;
  }

  return IsTileCollision( i_rcPos, i_pcPawn->GetMoveType() );
}


bool rumMap::IsWithinBounds( const rumPosition& i_rcPos ) const
{
  return !( i_rcPos.m_iX < 0 || i_rcPos.m_iY < 0 ||
            i_rcPos.m_iX >= (int32_t)m_uiCols || i_rcPos.m_iY >= (int32_t)m_uiRows );
}


bool rumMap::IsWithinRadius( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                             const float i_fRadius ) const
{
  // Early out if target and origin positions are the same
  if( i_rcTarget == i_rcOrigin )
  {
    return true;
  }

  // Note: This is very similar to the functionality provided by GetDistance(), but may result in much less work
  // because it will stop processing locations as soon as any locations are found to be within the specified radius

  // Test the provided position before resorting to map-wrapping
  bool bWithin{ IsWithinRadiusInternal( i_rcOrigin, i_rcTarget, i_fRadius ) };

  if( !bWithin && m_bWraps )
  {
    std::vector<rumPosition> vPositions;

    // Add all potential wrapped locations for the player
    AppendWrappedPositions( i_rcTarget, vPositions );

    // Visit each wrapped position, but discontinue early if the target position is within LOS
    for( uint32_t i = 0; !bWithin && i < vPositions.size(); ++i )
    {
      bWithin = IsWithinRadiusInternal( i_rcOrigin, vPositions[i], i_fRadius );
    }
  }

  return bWithin;
}


bool rumMap::IsWithinRadiusInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                                     float i_fRadius ) const
{
  // Early out if target and origin positions are the same
  if( i_rcTarget == i_rcOrigin )
  {
    return true;
  }

  // Get the difference between the two positions
  const uint32_t dx{ (uint32_t)( abs( i_rcTarget.m_iX - i_rcOrigin.m_iX ) ) };
  const uint32_t dy{ (uint32_t)( abs( i_rcTarget.m_iY - i_rcOrigin.m_iY ) ) };

  const double dt{ (double)( SQR( dx ) + SQR( dy ) ) };

  return ( dt <= (double)SQR( i_fRadius ) );
}


bool rumMap::IsWithinTileDistance( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                                   uint32_t i_uiMaxTileDistance, rumDirectionType i_eDir ) const
{
  // Early out if target and origin positions are the same
  if( i_rcTarget == i_rcOrigin )
  {
    return true;
  }

  // Note: This is very similar to the functionality provided by GetTileDistance(), but may result in much less work
  // because it will stop processing locations as soon as any locations are found to be within the specified distance

  // Test the provided position before resorting to map-wrapping
  bool bWithin{ IsWithinTileDistanceInternal( i_rcOrigin, i_rcTarget, i_uiMaxTileDistance, i_eDir ) };

  if( !bWithin && m_bWraps )
  {
    std::vector<rumPosition> vPositions;

    // Add all potential wrapped locations for the player
    AppendWrappedPositions( i_rcTarget, vPositions );

    // Visit each wrapped position, but discontinue early if the target position is within LOS
    for( uint32_t i = 0; !bWithin && i < vPositions.size(); ++i )
    {
      bWithin = IsWithinTileDistanceInternal( i_rcOrigin, vPositions[i], i_uiMaxTileDistance, i_eDir );
    }
  }

  return bWithin;
}


bool rumMap::IsWithinTileDistanceInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                                           uint32_t i_uiTileDistance, rumDirectionType i_eDir ) const
{
  // Early out if target and origin positions are the same
  if( i_rcTarget == i_rcOrigin )
  {
    return true;
  }

  // Get the difference between the two positions
  const uint32_t dx{ (uint32_t)( abs( i_rcTarget.m_iX - i_rcOrigin.m_iX ) ) };
  const uint32_t dy{ (uint32_t)( abs( i_rcTarget.m_iY - i_rcOrigin.m_iY ) ) };
  uint32_t dt{ 0 };

  if( Intercardinal_DirectionType == i_eDir )
  {
    // Number of steps between the two positions, diagonal movement allowed
    dt = std::max( dx, dy );
  }
  else
  {
    // Number of steps between the two positions, diagonal movement NOT allowed
    dt = dx + dy;
  }

  return ( dt <= i_uiTileDistance );
}


int32_t rumMap::Load()
{
  const std::string strInfo{ "Loading map " + GetName() };
  Logger::LogStandard( strInfo );

  // If anything was previously loaded, free it
  FreeCellData();

  assert( m_pcAsset );
  if( !m_pcAsset )
  {
    return RESULT_FAILED;
  }

  int32_t eResult{ RESULT_FAILED };
  std::string strPath;
  if( rumResource::FileExists( m_pcAsset->GetFilename() ) ||
      rumResource::FindFile( m_pcAsset->GetFilename(), strPath ) )
  {
    auto sqObject{ m_pcAsset->GetProperty( Map_Wraps_PropertyID ) };
    if( sqObject.GetType() == OT_BOOL )
    {
      m_bWraps = sqObject.Cast<bool>();
    }

    // Load tile data
    rumResourceLoader cResource;
    if( cResource.LoadFile( strPath ) )
    {
      eResult = Serialize( cResource );
    }

    if( RESULT_SUCCESS == eResult )
    {
      // Load pawn data. Note: Pawn files are not required to exist!
      std::filesystem::path cPath( strPath );
      cPath.replace_extension( ".pwn" );

      if( cResource.LoadFile( cPath.string() ) )
      {
        eResult = SerializePawns( cResource );
      }
    }
  }

  if( RESULT_SUCCESS == eResult )
  {
    OnLoaded();
  }

  return eResult;
}


// override
void rumMap::Manage()
{
  const rumUniqueID uiGameID{ GetGameID() };
  rumAssert( uiGameID != INVALID_GAME_ID );
  if( uiGameID != INVALID_GAME_ID )
  {
    s_hashMaps.insert( { uiGameID, this } );
  }
}


rumMoveResultType rumMap::MovePawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos, rumMoveFlags i_eMovementFlags,
                                    uint32_t i_uiTileDistance )
{
  if( !i_pcPawn )
  {
    return Error_MoveResultType;
  }

  if( !( i_eMovementFlags & IgnoreDistance_MoveFlag ) )
  {
    if( GetTileDistance( i_pcPawn->GetPos(), i_rcPos, Intercardinal_DirectionType ) > i_uiTileDistance )
    {
      return TooFar_MoveResultType;
    }
  }

  rumPosition cNewPos( i_rcPos );
  const rumPosition::PositionValidationEnum eStatus{ ValidatePosition( cNewPos ) };
  rumAssertMsg( eStatus != rumPosition::POSITION_UNCHECKED, "Position unchecked" );

  if( rumPosition::POSITION_OUT_OF_BOUNDS == eStatus )
  {
    return OffMap_MoveResultType;
  }

  rumAssertMsg( rumPosition::POSITION_OK == eStatus, "Position status != POSITION_OK" );

  rumMoveResultType eResult{ Success_MoveResultType };

  // Do the collision check
  const rumCollisionType eCollision{ IsCollision( cNewPos, i_pcPawn, i_eMovementFlags ) };
  switch( eCollision )
  {
    case Tile_CollisionType:  eResult = TileCollision_MoveResultType; break;
    case Pawn_CollisionType:  eResult = PawnCollision_MoveResultType; break;
    case Error_CollisionType: eResult = Error_MoveResultType; break;
  }

  // Make sure result did not change during collision-checked movement. Also, only update the pawn position if this
  // was not a test move
  if( ( Success_MoveResultType == eResult ) && !( i_eMovementFlags & Test_MoveFlag ) )
  {
    // Perform the actual movement
    PositionData* pcOriginCell{ AccessPositionData( i_pcPawn->GetPos() ) };
    PositionData* pcDestCell{ AccessPositionData( cNewPos ) };

    // This should never be null under normal circumstances since we've already validated the position
    rumAssertMsg( pcDestCell, "Destination cell is null" );

    // Place the pawn before removing, since the insertion might fail
    if( pcDestCell->InsertPawn( i_pcPawn ) )
    {
      // Remove from the previous position and update the pawn itself - there may not be a previous position if
      // this pawn was just placed
      if( pcOriginCell )
      {
        pcOriginCell->RemovePawn( i_pcPawn );
      }

      i_pcPawn->SetPos( cNewPos );
    }
    else
    {
      eResult = Error_MoveResultType;
    }
  }

  return eResult;
}


rumMoveResultType rumMap::OffsetPawn( rumPawn* i_pcPawn, const rumVector& i_vOffset, rumMoveFlags i_uiMovementFlags,
                                      uint32_t i_uiTileDistance )
{
  const rumPosition cPos( i_pcPawn->GetPosX() + (int32_t)i_vOffset.m_fX,
                          i_pcPawn->GetPosY() + (int32_t)i_vOffset.m_fY );
  return MovePawn( i_pcPawn, cPos, i_uiMovementFlags, i_uiTileDistance );
}


// override
void rumMap::OnCreated()
{
  super::OnCreated();

  if( !IsLoaded() )
  {
    Load();
  }
}


// virtual
void rumMap::OnLoaded()
{
  rumScript::ExecOptionalFunc( GetScriptInstance(), "OnLoaded" );
}


bool rumMap::PositionBlocksSight( const rumPosition& i_rcPos ) const
{
  bool bBlocked{ false };

  rumPosition cPos( i_rcPos );
  ValidatePosition( cPos );

  // See if the base tile blocks sight
  const rumTileAsset* pcTile{ rumTileAsset::Fetch( GetTileID( cPos ) ) };
  if( pcTile )
  {
    bBlocked = pcTile->GetBlocksLOS();
  }

  // See if a map pawn blocks sight
  if( !bBlocked )
  {
    Sqrat::Object sqValue;
    rumPawn* pcPawn{ nullptr };
    rumPositionIterator iter( GetPositionIterator( cPos ) );
    while( !bBlocked && !iter.Done() )
    {
      pcPawn = iter.GetNextObjectPtr();
      if( pcPawn )
      {
        bBlocked = pcPawn->IsVisible() && pcPawn->GetBlocksLOS();
      }
    }
  }

  return bBlocked;
}


bool rumMap::RemovePawn( rumPawn* i_pcPawn )
{
  if( !i_pcPawn )
  {
    return false;
  }

  bool bRemoved{ false };

  // Remove reference from map cell
  PositionData* pcPosition{ AccessPositionData( i_pcPawn->GetPos() ) };
  if( pcPosition )
  {
    bRemoved = pcPosition->RemovePawn( i_pcPawn );
  }

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  rumScript::ExecOptionalFunc( i_pcPawn->GetScriptInstance(), "OnPawnRemoved" );

  // Remove from main pawn container
  m_hashPawns.erase( i_pcPawn->GetGameID() );

  // The pawn no longer belongs to any map
  i_pcPawn->SetMapID( INVALID_GAME_ID, rumPosition( 0, 0 ) );

  // Remove players from the map as well
  if( i_pcPawn->IsPlayer() )
  {
    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( i_pcPawn->GetPlayerID() ) };
    if( pcPlayer )
    {
      RemovePlayer( pcPlayer );
    }
  }

  return bRemoved;
}


bool rumMap::RemovePawnVM( Sqrat::Object i_sqObject )
{
  bool bRemoved{ false };

  if( i_sqObject.GetType() == OT_INSTANCE )
  {
    rumPawn* pcPawn{ i_sqObject.Cast<rumPawn*>() };
    if( pcPawn )
    {
      bRemoved = RemovePawn( pcPawn );
      if( bRemoved )
      {
        // Release the script reference
        UnmanageScriptObject( i_sqObject );
      }
    }
  }

  return bRemoved;
}


int32_t rumMap::RemovePlayer( rumPlayer* i_pcPlayer )
{
  if( !i_pcPlayer )
  {
    return RESULT_FAILED;
  }

  std::string strInfo{ "Removing player " };
  strInfo += i_pcPlayer->GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( i_pcPlayer->GetPlayerID() );
  strInfo += "] from Map ";
  strInfo += GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( GetGameID() );
  strInfo += "]";
  Logger::LogStandard( strInfo );

  m_hashPlayers.erase( i_pcPlayer->GetPlayerID() );
  RUM_COUT( "Map has " << m_hashPlayers.size() << " player(s)" << '\n' );

  rumPawn* pcPawn{ i_pcPlayer->GetPlayerPawn() };
  if( pcPawn )
  {
    // Notify scripts that the player has been removed from the map
    rumScript::ExecOptionalFunc( GetScriptInstance(), "OnPlayerRemoved", pcPawn->GetScriptInstance() );
  }

  return RESULT_SUCCESS;
}


void rumMap::ScriptBind()
{
  rumPositionIterator::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumMap, rumGameObject, Sqrat::NoConstructor<rumMap>> cMap( pcVM, "rumMap" );
  cMap
    .Func( "AddPawn", &AddPawnVM )
    .Func( "GetNumColumns", &GetCols )
    .Func( "GetNumRows", &GetRows )
    .Func( "GetDistance", &GetDistance )
    .Func( "GetExitMapID", &GetExitMapID )
    .Func( "GetExitPosition", &GetExitPos )
    .Func( "GetBorderTile", &GetBorderTile )
    .Func( "SetBorderTile", &SetBorderTile )
    .Func( "GetDirectionVector", &GetDirectionVector )
    .Func( "GetAllPawns", &ScriptGetAllPawns )
    .Func( "GetAllPlayers", &ScriptGetAllPlayers )
    .Func( "GetNumPlayers", &GetNumPlayers )
    .Func( "GetPawns", &ScriptGetPawns )
    .Func( "GetPlayers", &ScriptGetPlayers )
    .Func( "HasClearPath", &HasClearPath )
    .Func( "IsPositionWithinBounds", &IsWithinBounds )
    .Func( "IsPositionWithinRadius", &IsWithinRadius )
    .Func( "RemovePawn", &RemovePawnVM )
    .Func( "SupportsWrapping", &Wraps )
    .Overload<rumPlayer* ( rumMap::* )( const rumPosition& ) const>( "GetNearestPlayer", &GetNearestPlayer )
    .Overload<rumPlayer* ( rumMap::* )( const rumPosition&, uint32_t ) const>( "GetNearestPlayer", &GetNearestPlayer )
    .Overload<rumPlayer* ( rumMap::* )( const rumPosition&, uint32_t, rumDirectionType ) const>( "GetNearestPlayer", &GetNearestPlayer )
    .Overload<rumPlayer* ( rumMap::* )( const rumPosition&, uint32_t, rumDirectionType, bool ) const>( "GetNearestPlayer", &GetNearestPlayer )
    .Overload<bool( rumMap::* )( const rumPosition&, const rumPosition&, uint32_t, rumDirectionType ) const>( "IsPositionWithinTileDistance", &IsWithinTileDistance )
    .Overload<bool( rumMap::* )( const rumPosition&, const rumPosition&, uint32_t ) const>( "IsPositionWithinTileDistance", &IsWithinTileDistance )
    .Overload<uint32_t( rumMap::* )( const rumPosition&, const rumPosition& ) const>( "GetTileDistance", &GetTileDistance )
    .Overload<uint32_t( rumMap::* )( const rumPosition&, const rumPosition&, rumDirectionType ) const>( "GetTileDistance", &GetTileDistance )
    .Overload<bool( rumMap::* )( const rumPosition&, const rumPosition&, uint32_t, rumDirectionType ) const>( "TestLOS", &HasLineOfSight )
    .Overload<bool( rumMap::* )( const rumPosition&, const rumPosition&, uint32_t ) const>( "TestLOS", &HasLineOfSight )
    .Overload<bool( rumMap::* )( const rumPosition&, const rumPosition& ) const>( "TestLOS", &HasLineOfSight )
    .Overload<rumPositionIterator( rumMap::* )( const rumPosition& ) const>( "GetPositionData", &GetPositionIterator )
    .Overload<rumPositionIterator( rumMap::* )( int32_t, int32_t ) const>( "GetPositionData", &GetPositionIterator )
    .Overload<bool( rumMap::* )( rumPawn*, rumMap*, const rumPosition& )>( "TransferPawn", &TransferPawn )
    .Overload<bool( rumMap::* )( rumPawn*, rumMap*, const rumPosition&, bool )>( "TransferPawn", &TransferPawn )
    .Overload<rumMoveResultType( rumMap::* )( rumPawn*, const rumPosition&, rumMoveFlags, uint32_t )>( "MovePawn", &MovePawn )
    .Overload<rumMoveResultType( rumMap::* )( rumPawn*, const rumPosition&, rumMoveFlags )>( "MovePawn", &MovePawn )
    .Overload<rumMoveResultType( rumMap::* )( rumPawn*, const rumPosition& )>( "MovePawn", &MovePawn )
    .Overload<rumMoveResultType( rumMap::* )( rumPawn*, const rumVector&, rumMoveFlags, uint32_t )>( "OffsetPawn", &OffsetPawn )
    .Overload<rumMoveResultType( rumMap::* )( rumPawn*, const rumVector&, rumMoveFlags )>( "OffsetPawn", &OffsetPawn )
    .Overload<rumMoveResultType( rumMap::* )( rumPawn*, const rumVector& )>( "OffsetPawn", &OffsetPawn );
  Sqrat::RootTable( pcVM ).Bind( "rumMapBase", cMap );

  Sqrat::RootTable( pcVM ).Func( "rumGetMap", Fetch );
}


Sqrat::Object rumMap::ScriptGetAllPawns()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Array sqArray( pcVM, m_hashPawns.size() );
  int32_t iIndex{ 0 };

  // Visit all pawns
  for( const auto iter : m_hashPawns )
  {
    const rumUniqueID uiPawnID{ iter };
    rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
    if( pcPawn )
    {
      sqArray.SetValue( iIndex++, pcPawn->GetScriptInstance() );
    }
  }

  return sqArray;
}


Sqrat::Object rumMap::ScriptGetAllPlayers()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Array sqArray( pcVM, m_hashPlayers.size() );
  int32_t iIndex{ 0 };

  // Visit all players
  for( const auto iter : m_hashPlayers )
  {
    const rumUniqueID uiPlayerID{ iter };

    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( uiPlayerID ) };
    if( pcPlayer )
    {
      rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
      if( pcPawn )
      {
        sqArray.SetValue( iIndex++, pcPawn->GetScriptInstance() );
      }
    }
  }

  return sqArray;
}


Sqrat::Object rumMap::ScriptGetPawns( const rumPosition& i_rcOrigin, const uint32_t i_uiMaxTileDistance,
                                      const bool i_bCheckLOS )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Array sqArray( pcVM, m_hashPawns.size() );
  uint32_t iIndex{ 0 };

  // Visit all pawns
  for( const auto iter : m_hashPawns )
  {
    const rumUniqueID uiPawnID{ iter };
    rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
    if( pcPawn )
    {
      bool bVisible{ false };

      if( i_bCheckLOS )
      {
        bVisible = HasLineOfSight( i_rcOrigin, pcPawn->GetPos(), i_uiMaxTileDistance, Intercardinal_DirectionType );
      }
      else
      {
        bVisible = IsWithinTileDistance( i_rcOrigin, pcPawn->GetPos(), i_uiMaxTileDistance,
                                         Intercardinal_DirectionType );
      }

      if( bVisible )
      {
        sqArray.SetValue( iIndex++, pcPawn->GetScriptInstance() );
      }
    }
  }

  if( iIndex < m_hashPawns.size() )
  {
    // Reduce the array size to the number of pawns that were inserted
    sqArray.Resize( iIndex );
  }

  return sqArray;
}


Sqrat::Object rumMap::ScriptGetPlayers( const rumPosition& i_rcPos, const uint32_t i_uiMaxTileDistance,
                                        const bool i_bCheckLOS )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Array sqArray( pcVM, m_hashPlayers.size() );
  uint32_t iIndex{ 0 };

  // Visit all players
  for( const auto iter : m_hashPlayers )
  {
    const rumUniqueID uiPlayerID{ iter };

    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( uiPlayerID ) };
    if( pcPlayer )
    {
      rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
      if( pcPawn )
      {
        bool bVisible{ false };

        if( i_bCheckLOS )
        {
          bVisible = HasLineOfSight( i_rcPos, pcPawn->GetPos(), i_uiMaxTileDistance, Intercardinal_DirectionType );
        }
        else
        {
          bVisible = IsWithinTileDistance( i_rcPos, pcPawn->GetPos(), i_uiMaxTileDistance,
                                           Intercardinal_DirectionType );
        }

        if( bVisible )
        {
          sqArray.SetValue( iIndex++, pcPawn->GetScriptInstance() );
        }
      }
    }
  }

  if( iIndex < m_hashPlayers.size() )
  {
    // Reduce the array size to the number of pawns that were inserted
    sqArray.Resize( iIndex );
  }

  return sqArray;
}


// virtual
int32_t rumMap::Serialize( rumResource& io_rcResource )
{
  int32_t eResult{ RESULT_SUCCESS };

  // Version numbers
  rumWord nativeVersion{ 1 };

  io_rcResource << nativeVersion;

  // Read in the number of tiles stored
  io_rcResource << (rumDWord&)m_uiRows;
  io_rcResource << (rumDWord&)m_uiCols;

  rumDWord eTileID{ 0 };
  if( io_rcResource.IsLoading() )
  {
    m_eExitMapID = 0;
    m_uiMapSize = m_uiRows * m_uiCols;

    // Allocate map in two dimensions
    m_pcData = new PositionData * [m_uiRows];
    if( m_pcData )
    {
      for( uint32_t i = 0; i < m_uiRows; ++i )
      {
        m_pcData[i] = new PositionData[m_uiCols];
      }
    }

    // Load all map tiles
    for( uint32_t j = 0; j < m_uiRows; ++j )
    {
      for( uint32_t i = 0; i < m_uiCols; ++i )
      {
        io_rcResource << (rumDWord)eTileID;
        m_pcData[j][i].m_eTileID = eTileID;
      }
    }

    // Read the border tile
    io_rcResource << (rumDWord)eTileID;
    m_eBorderTileID = eTileID;

    io_rcResource << (rumDWord&)m_eExitMapID;

    io_rcResource << (rumDWord&)m_cExitPos.m_iX;
    io_rcResource << (rumDWord&)m_cExitPos.m_iY;
  }
  else // saving
  {
    // Save all map tiles
    for( uint32_t j = 0; j < m_uiRows; ++j )
    {
      for( uint32_t i = 0; i < m_uiCols; ++i )
      {
        eTileID = m_pcData[j][i].m_eTileID;
        io_rcResource << (rumDWord)eTileID;
      }
    }

    eTileID = m_eBorderTileID;
    io_rcResource << (rumDWord)eTileID;

    io_rcResource << (rumDWord)m_eExitMapID;
    io_rcResource << (rumDWord)m_cExitPos.m_iX;
    io_rcResource << (rumDWord)m_cExitPos.m_iY;
  }

  return eResult;
}


// virtual
int32_t rumMap::SerializePawns( rumResource& io_rcResource )
{
  uint32_t uiNumSerialized{ 0 };

  int32_t eResult{ RESULT_SUCCESS };

  // Version numbers
  rumWord nativeVersion{ 1 };
  io_rcResource << nativeVersion;

  if( io_rcResource.IsSaving() )
  {
    // Since we want the pawn list to be consistent on every export, step through the map from top to bottom one tile
    // at a time and serialize the pawns in each cell. This also allows designers to specify the draw order of stacked
    // tiles in the editor and have that same order reflected in-game.
    for( uint32_t j = 0; j < m_uiRows; ++j )
    {
      for( uint32_t i = 0; i < m_uiCols; ++i )
      {
        m_pcData[j][i].m_cPawnDataList.reverse();

        for( const auto& rcPawnData : m_pcData[j][i].m_cPawnDataList )
        {
          const rumUniqueID uiPawnID{ rcPawnData.m_iPawnID };

          rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
          if( pcPawn )
          {
            // The pawn type can be inferred from the asset
            io_rcResource << (rumDWord)pcPawn->GetAssetID();
          
            if( pcPawn->Serialize( io_rcResource ) != RESULT_SUCCESS )
            {
              std::string strError{ "Failed to serialize pawn asset [" };
              strError += rumStringUtils::ToHexString( pcPawn->GetAssetID() );
              strError += "], offset ";
              strError += rumStringUtils::ToString( (int32_t)io_rcResource.GetPos() );
              Logger::LogStandard( strError );
              return RESULT_FAILED;
            }
          
            ++uiNumSerialized;
          }
        }
      }
    }

    RUM_COUT( "------------------------------------------------------------------------------\n" );
    RUM_COUT( "Saved " << uiNumSerialized << " pawns\n" );
    RUM_COUT( "------------------------------------------------------------------------------\n" );
  }
  if( io_rcResource.IsLoading() && !io_rcResource.IsEndOfFile() )
  {
    uint32_t uiNumCreatures{ 0 };
    uint32_t uiNumWidgets{ 0 };
    uint32_t uiNumPortals{ 0 };

    static_assert( sizeof( rumAssetID ) == sizeof( rumDWord ) );

    rumAssetID eAssetID{ INVALID_ASSET_ID };
    io_rcResource << eAssetID;

    while( ( RESULT_SUCCESS == eResult ) && !io_rcResource.IsEndOfFile() )
    {
      eResult = RESULT_FAILED;

#if MEMORY_DEBUG
      const rumPawnAsset* pcAsset{ rumPawnAsset::Fetch( eAssetID ) };
      if( !pcAsset )
      {
        std::string strError{ "Unknown pawn encountered [" };
        strError += rumStringUtils::ToHexString( eAssetID );
        strError += "]. Pawn loading aborted.";
        Logger::LogStandard( strError );
        return RESULT_FAILED;
      }

      const std::string& strName{ pcAsset->GetName() };
      RUM_COUT( strName << " [" << rumStringUtils::ToHexString( eAssetID ) << "], offset " <<
                io_rcResource.GetPos() << '\n' );
#endif // MEMORY DEBUG

      // Create an instance of the script type we just read
      Sqrat::Object sqInstance{ rumGameObject::Create( eAssetID ) };
      if( sqInstance.GetType() == OT_INSTANCE )
      {
#if MEMORY_DEBUG
        // There should only be 1 ref at this point!
        rumAssertMsg( rumScript::GetObjectRefCount( sqInstance ) == 1,
                      "Object ref count != 1 for asset " << strName << ", offset " << io_rcResource.pos() );
#endif // MEMORY DEBUG

        bool bAdded{ false };

        // Fully load the pawn from the resource
        rumPawn* pcPawn{ sqInstance.Cast<rumPawn*>() };
        if( pcPawn )
        {
          if( pcPawn->Serialize( io_rcResource ) == RESULT_SUCCESS )
          {
            if( AddPawn( pcPawn, pcPawn->GetPos() ) )
            {
              ++uiNumSerialized;
              switch( RAW_ASSET_TYPE( eAssetID ) )
              {
                case AssetType::Creature_AssetType: ++uiNumCreatures; break;
                case AssetType::Portal_AssetType:   ++uiNumPortals;   break;
                case AssetType::Widget_AssetType:   ++uiNumWidgets;   break;
                default:
                  rumAssertMsg( false, "Unsupported AssetType" );
                  break;
              }

              bAdded = true;
              eResult = RESULT_SUCCESS;
            }
          }

          if( !bAdded )
          {
            pcPawn->Free();

            std::string strError{ "Failed to serialize pawn asset [" };
            strError += rumStringUtils::ToHexString( eAssetID );
            strError += "], offset ";
            strError += rumStringUtils::ToString( (int32_t)io_rcResource.GetPos() );
            Logger::LogStandard( strError );
            return RESULT_FAILED;
          }

          ManageScriptObject( sqInstance );
        }
      }

      // See if another object exists in the file
      io_rcResource << eAssetID;
    }

    //cout << "Exit stack size = " << sq_gettop(SquirrelVM::GetVMPtr()) << std::endl;

    if( RESULT_SUCCESS == eResult )
    {
      RUM_COUT( "------------------------------------------------------------------------------\n" );
      RUM_COUT( "Loaded    " << uiNumSerialized << " pawns\n" );
      RUM_COUT( "------------------------------------------------------------------------------\n" );
      RUM_COUT( "Creatures " << uiNumCreatures << '\n' );
      RUM_COUT( "Widgets   " << uiNumWidgets << '\n' );
      RUM_COUT( "Portals   " << uiNumPortals << '\n' );
      RUM_COUT( "------------------------------------------------------------------------------\n" );
    }
    else
    {
      std::string strError{ "Error encountered at file offset " };
      strError += rumStringUtils::ToString( (int32_t)io_rcResource.GetPos() );

      RUM_COUT( "------------------------------------------------------------------------------\n" );
      Logger::LogStandard( strError );
      RUM_COUT( "------------------------------------------------------------------------------\n" );
    }
  }

  return RESULT_SUCCESS;
}


// static
void rumMap::Shutdown()
{
  rumAssert( s_hashMaps.empty() );
  if( !s_hashMaps.empty() )
  {
    // Make a copy of the hash map for iteration and deletion. We can't delete directly from the original hash because
    // the iteration will be destroyed by calls to each object's Free method.
    MapHash cHash{ s_hashMaps };
    s_hashMaps.clear();

    for( const auto& iter : cHash )
    {
      RUM_COUT_IFDEF( MEMORY_DEBUG, "Freeing Map: " << iter.second->GetName() << '\n' );

      rumMap* pcObject{ iter.second };
      if( pcObject )
      {
        pcObject->Free();
      }
    }

    cHash.clear();
  }
}


bool rumMap::TransferPawn( rumPawn* i_pcPawn, rumMap* i_pcMap, const rumPosition& i_rcPos, bool i_bForce )
{
  rumAssert( i_pcPawn && i_pcMap );
  if( !i_pcPawn || !i_pcMap )
  {
    return false;
  }

  bool bSuccess{ false };

  const bool bNeedsTransfer{ i_pcMap != this };
  if( bNeedsTransfer || i_bForce )
  {
    const rumPosition cOldPosition{ i_pcPawn->GetPos() };
    rumMap* pcSourceMap{ i_pcPawn->GetMap() };

    // Make sure the destination map is ready to go
    // Remove the pawn from its current map if necessary
    if( ( bNeedsTransfer && RemovePawn( i_pcPawn ) ) || i_bForce )
    {
      // Add the pawn to the destination map
      bSuccess = i_pcMap->AddPawn( i_pcPawn, i_rcPos );
      if( !bSuccess && pcSourceMap )
      {
        // Put the pawn back where it came from
        pcSourceMap->AddPawn( i_pcPawn, cOldPosition );
      }
    }
  }
  else
  {
    // The pawn is already on the specified map & instance, just force-move it to the new position
    const rumMoveFlags eMoveFlags{ (rumMoveFlags)( IgnoreTileCollision_MoveFlag | IgnorePawnCollision_MoveFlag |
                                                   IgnoreDistance_MoveFlag ) };
    bSuccess = MovePawn( i_pcPawn, i_rcPos, eMoveFlags, 0 ) == Success_MoveResultType;
  }

  return bSuccess;
}


// override
void rumMap::Unmanage()
{
  s_hashMaps.erase( GetGameID() );
}


rumPosition::PositionValidationEnum rumMap::ValidatePosition( rumPosition& io_rcPos ) const
{
  if( rumPosition::POSITION_UNCHECKED == io_rcPos.m_eStatus )
  {
    rumPosition::PositionValidationEnum eStatus{ rumPosition::POSITION_OK };

    // Check bounds
    if( !IsWithinBounds( io_rcPos ) )
    {
      if( m_bWraps )
      {
        WrapPosition( io_rcPos );
      }
      else
      {
        eStatus = rumPosition::POSITION_OUT_OF_BOUNDS;
      }
    }

    // Update status
    io_rcPos.m_eStatus = eStatus;
  }

  return io_rcPos.m_eStatus;
}


void rumMap::WrapPosition( rumPosition& io_rcPos ) const
{
  if( io_rcPos.m_iX < 0 )
  {
    io_rcPos.m_iX += m_uiCols;
  }
  else if( io_rcPos.m_iX >= (int32_t)m_uiCols )
  {
    io_rcPos.m_iX -= m_uiCols;
  }

  if( io_rcPos.m_iY < 0 )
  {
    io_rcPos.m_iY += m_uiRows;
  }
  else if( io_rcPos.m_iY >= (int32_t)m_uiRows )
  {
    io_rcPos.m_iY -= m_uiRows;
  }
}


bool rumMap::PositionData::InsertPawn( rumPawn* i_pcPawn )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return false;
  }

  const float fDrawOrder{ i_pcPawn->GetDrawOrder() };

  // Find where this pawn can be inserted
  PawnDataList::iterator iter( m_cPawnDataList.begin() );
  const PawnDataList::iterator end( m_cPawnDataList.end() );
  while( iter != end && fDrawOrder < iter->m_fDrawOrder )
  {
    ++iter;
  }

  // Create pawn data to represent the pawn for this position
  PawnData cPawnData( fDrawOrder, i_pcPawn->GetGameID() );

  // Add the pawn to the map cell
  return( m_cPawnDataList.insert( iter, cPawnData ) != m_cPawnDataList.end() );
}


bool rumMap::PositionData::RemovePawn( rumPawn* i_pcPawn )
{
  PawnDataList::iterator iter( m_cPawnDataList.begin() );
  const PawnDataList::iterator end( m_cPawnDataList.end() );
  while( iter != end && iter->m_iPawnID != i_pcPawn->GetGameID() )
  {
    ++iter;
  }

  if( iter != end )
  {
    m_cPawnDataList.erase( iter );
    return true;
  }

  return false;
}


void rumMap::PositionData::ShiftPawnsDown()
{
  // Move the pawn at the back of the list to the front of the list
  if( m_cPawnDataList.size() > 1 )
  {
    // Move the pawn at the front of the list to the back of the list
    PawnDataList::reverse_iterator iter( m_cPawnDataList.rbegin() );
    if( iter != m_cPawnDataList.rend() )
    {
      PawnData cPawn( *iter );
      m_cPawnDataList.pop_back();
      m_cPawnDataList.push_front( cPawn );
    }
  }
}


void rumMap::PositionData::ShiftPawnsUp()
{
  if( m_cPawnDataList.size() > 1 )
  {
    // Move the pawn at the front of the list to the back of the list
    PawnDataList::iterator iter( m_cPawnDataList.begin() );
    if( iter != m_cPawnDataList.end() )
    {
      PawnData cPawn( *iter );
      m_cPawnDataList.pop_front();
      m_cPawnDataList.push_back( cPawn );
    }
  }
}
