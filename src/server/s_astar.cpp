#include <s_astar.h>

#include <u_assert.h>
#include <u_enum.h>
#include <u_pawn.h>
#include <u_tile_asset.h>

#if PATHING_DEBUG
#include <u_utility.h>
std::string g_strPathReport;
#endif


rumAstarPath::~rumAstarPath()
{
  FreePathNodes();
}


bool rumAstarPath::FindNewPath( rumPawn* io_pcPawn, const rumPosition& i_rcPos, const rumMap* i_pcMap,
                                const uint32_t i_uiMaxTileDistance, const float i_fMaxCost, rumDirectionType i_eDir )
{
  rumAssertMsg( i_pcMap, "Request to find Astar path on a null map" );

  if( !io_pcPawn || !i_pcMap )
  {
    return false;
  }

  // Forget the previous path
  FreePathNodes();

  m_bPathComplete = false;

  // Save the pawn's position as the point we are trying to search back to
  m_cOriginPos = io_pcPawn->GetPos();
  m_cTargetPos = i_rcPos;

#if PATHING_DEBUG
  g_strPathReport = "Creating path for " + io_pcPawn->GetName();
  g_strPathReport += "\nOrigin position: " + m_cOriginPos.ToString();
  g_strPathReport += "\nTarget position: " + m_cTargetPos.ToString();
  g_strPathReport += "\nMax distance: ";
  g_strPathReport += rumStringUtils::ToString( i_uiMaxTileDistance );
  g_strPathReport += "\nMax cost: ";
  g_strPathReport += rumStringUtils::ToFloatString( i_fMaxCost );
#endif // PATHING_DEBUG

  if( i_rcPos == m_cOriginPos )
  {
    m_bPathComplete = true;
  }
  else if( i_pcMap && i_pcMap->IsWithinTileDistance( io_pcPawn->GetPos(), i_rcPos, i_uiMaxTileDistance, i_eDir ) )
  {
    // Find a path from start to goal
    FindPath( i_rcPos, i_pcMap, io_pcPawn, i_fMaxCost, i_eDir );
  }

#if PATHING_DEBUG
  g_strPathReport += "\nPath found: ";
  g_strPathReport += ( m_bPathComplete ? "true" : "false" );
#endif

  return m_bPathComplete;
}


rumAstarPath::rumPathNode* rumAstarPath::CheckClosedPath( const rumPosition& i_rcPos )
{
  // Find and return any existing node at the specified position
  rumPathNode* pcNode{ m_pcClosedList->m_pcNext };
  while( pcNode && pcNode->m_cPos != i_rcPos )
  {
    pcNode = pcNode->m_pcNext;
  }

  return pcNode;
}


rumAstarPath::rumPathNode* rumAstarPath::CheckOpenPath( const rumPosition& i_rcPos )
{
  // Find and return any existing node at the specified position
  rumPathNode* pcNode{ m_pcOpenList };
  while( pcNode && pcNode->m_cPos != i_rcPos )
  {
    pcNode = pcNode->m_pcNext;
  }

  return pcNode;
}


void rumAstarPath::CheckPosition( rumPathNode* io_pBestNode, rumPosition& io_rcPos, const rumMap* i_pcMap,
                                  rumPawn* io_pcPawn, const float i_fMaxMoveCost, const float i_fMoveCost )
{
  // If the position is our goal or if the position is free of colliding obstacles, generate a path node
  if( ( m_cOriginPos == io_rcPos ) ||
      ( ( i_pcMap->IsCollision( io_rcPos, io_pcPawn, Test_MoveFlag ) == None_CollisionType ) &&
        !i_pcMap->IsHarmful( io_rcPos, io_pcPawn ) ) )
  {
    float fWeight{ 0.f };

    const rumTileAsset* pcTile{ rumTileAsset::Fetch( i_pcMap->GetTileID( io_rcPos ) ) };
    if( pcTile )
    {
      fWeight = pcTile->GetWeight();
    }

    GenerateSuccessorNode( io_pBestNode, io_rcPos, i_fMaxMoveCost, i_fMoveCost + fWeight );
  }
}


void rumAstarPath::FindPath( const rumPosition& io_rcPos, const rumMap* i_pcMap, rumPawn* io_pcPawn,
                             const float i_fMaxCost, const rumDirectionType i_eDir )
{
  // Do the search backwards so that we can follow the nodes towards the destination when the search is done.
  // Create starting node at the destination and work back towards the source.
  rumPathNode* pcNode{ new rumPathNode };
  if( pcNode )
  {
    pcNode->m_fGScore = 0;

    // Using manhattan distance
    pcNode->m_uiHScore = abs( io_rcPos.m_iX - m_cOriginPos.m_iX ) + abs( io_rcPos.m_iY - m_cOriginPos.m_iY );
    pcNode->m_fFScore = (float)pcNode->m_uiHScore; //g + h, but g is always zero on first node
    pcNode->m_cPos = io_rcPos;

    // At this point, the starting node is the best node. Add it to the closed list.
    rumPathNode* pcBestNode{ pcNode };
    m_pcClosedList = pcNode;

    // Check for null (no path) or goal reached
    while( pcBestNode && !m_bPathComplete )
    {
      // Generate seekers from the destination node to the source node
      if( Intercardinal_DirectionType == i_eDir )
      {
        GenerateSuccessors8( pcBestNode, i_pcMap, io_pcPawn, i_fMaxCost );
      }
      else
      {
        GenerateSuccessors4( pcBestNode, i_pcMap, io_pcPawn, i_fMaxCost );
      }

      pcBestNode = GetBestNode();

#if PATHING_DEBUG
      if( pcBestNode )
      {
        g_strPathReport += "\nNew best node: " + pcBestNode->m_cPos.ToString();
      }
#endif // PATHING_DEBUG
    }

    // Step one node into path, since last node is where we are currently standing
    if( pcBestNode )
    {
      m_pcPathList = pcBestNode->m_pcParent;
    }
  }
}


void rumAstarPath::FreePathNodes()
{
  // Clear open list
  rumPathNode* pcNode{ m_pcOpenList };
  while( pcNode )
  {
    m_pcOpenList = m_pcOpenList->m_pcNext;
    delete pcNode;
    pcNode = m_pcOpenList;
  }

  // Clear closed list
  pcNode = m_pcClosedList;
  while( pcNode )
  {
    m_pcClosedList = m_pcClosedList->m_pcNext;
    delete pcNode;
    pcNode = m_pcClosedList;
  }

  m_pcOpenList = m_pcClosedList = m_pcPathList = nullptr;
}


void rumAstarPath::GenerateSuccessorNode( rumPathNode* io_pcParentNode, const rumPosition& i_rcPos,
                                          const float i_fMaxMoveCost, const float i_fMoveCost )
{
  // If cost of travel is getting too high, stop looking in this direction
  const float fGScore{ io_pcParentNode->m_fGScore + i_fMoveCost };

  // Only continue if cost is low and current tile is not on the closed list
  if( fGScore <= i_fMaxMoveCost && !CheckClosedPath( i_rcPos ) )
  {
    // See if tile is already on the open list
    rumPathNode* pcNode{ CheckOpenPath( i_rcPos ) };
    if( !pcNode )
    {
      // Not in the open list, so create the node
      rumPathNode* pcSuccessor{ new rumPathNode };
      if( pcSuccessor )
      {
        // Make the current square the parent of this square
        pcSuccessor->m_pcParent = io_pcParentNode;

        // Record the F, G, and H costs of the square
        pcSuccessor->m_fGScore  = fGScore;
        pcSuccessor->m_uiHScore = abs( i_rcPos.m_iX - m_cOriginPos.m_iX ) + abs( i_rcPos.m_iY - m_cOriginPos.m_iY );
        pcSuccessor->m_fFScore  = pcSuccessor->m_fGScore + pcSuccessor->m_uiHScore;
        pcSuccessor->m_cPos     = i_rcPos;

        // Add successor to the open list
        InsertPathNode( pcSuccessor );
      }
    }
    else
    {
      // Check to see if this path to that square is better, using G cost as the measure. A lower G cost means that
      // this is a better path. If so, change the parent of the square to the current square, and recalculate the G and
      // F scores of the square.
      if( fGScore < pcNode->m_fGScore )
      {
        pcNode->m_pcParent = io_pcParentNode;
        pcNode->m_fGScore  = fGScore;
        pcNode->m_uiHScore = abs( i_rcPos.m_iX - m_cOriginPos.m_iX ) + abs( i_rcPos.m_iY - m_cOriginPos.m_iY );
        pcNode->m_fFScore  = pcNode->m_fGScore + pcNode->m_uiHScore;
      }
    }
  }
}


void rumAstarPath::GenerateSuccessors4( rumPathNode* io_pcBestNode, const rumMap* i_pcMap, rumPawn* io_pcPawn,
                                        const float i_fMaxMoveCost )
{
  rumPosition cPos( io_pcBestNode->m_cPos );

  // North
  cPos.m_iY -= 1;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.f );

  // South
  cPos.m_iY += 2;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.f );

  // West
  cPos.m_iY = io_pcBestNode->m_cPos.m_iY;
  cPos.m_iX -= 1;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.f );

  // East
  cPos.m_iX += 2;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.f );
}


void rumAstarPath::GenerateSuccessors8( rumPathNode* io_pcBestNode, const rumMap* i_pcMap, rumPawn* io_pcPawn,
                                        const float i_fMaxMoveCost )
{
  // Handle cardinal directions first
  GenerateSuccessors4( io_pcBestNode, i_pcMap, io_pcPawn, i_fMaxMoveCost );

  rumPosition cPos( io_pcBestNode->m_cPos );

  // NorthEast
  cPos.m_iY -= 1;
  cPos.m_iX += 1;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.1f );

  // SouthEast
  cPos.m_iY += 2;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.1f );

  // SouthWest
  cPos.m_iX -= 2;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.1f );

  // NorthWest
  cPos.m_iY -= 2;
  CheckPosition( io_pcBestNode, cPos, i_pcMap, io_pcPawn, i_fMaxMoveCost, 1.1f );
}


rumAstarPath::rumPathNode *rumAstarPath::GetBestNode()
{
  rumPathNode* pcBestNode{ m_pcOpenList };
  if( pcBestNode )
  {
    rumPathNode* pcNode{ m_pcOpenList->m_pcNext };
    while( pcNode )
    {
      // See if the new node has a lower cost
      if( pcNode->m_fFScore < pcBestNode->m_fFScore )
      {
        pcBestNode = pcNode;
      }

      pcNode = pcNode->m_pcNext;
    }

    // Remove best from the open list
    if( !pcBestNode->m_pcPrev )
    {
      m_pcOpenList = pcBestNode->m_pcNext;
      if( m_pcOpenList )
      {
        m_pcOpenList->m_pcPrev = nullptr;
      }
    }
    else
    {
      pcBestNode->m_pcPrev->m_pcNext = pcBestNode->m_pcNext;
      if( pcBestNode->m_pcNext )
      {
        pcBestNode->m_pcNext->m_pcPrev = pcBestNode->m_pcPrev;
      }
    }

    // Add best to the front of the closed list (note that there is no need to book-keep prev nodes on the closed list)
    pcBestNode->m_pcNext = m_pcClosedList;
    m_pcClosedList = pcBestNode;

    // If we just closed the goal, a path has been found
    if( pcBestNode->m_cPos == m_cOriginPos )
    {
      m_bPathComplete = true;
    }
  }

  return pcBestNode;
}


void rumAstarPath::InsertPathNode( rumPathNode* io_pcSuccessorNode )
{
  io_pcSuccessorNode->m_pcNext = m_pcOpenList;
  if( m_pcOpenList )
  {
    m_pcOpenList->m_pcPrev = io_pcSuccessorNode;
  }

  m_pcOpenList = io_pcSuccessorNode;
}
