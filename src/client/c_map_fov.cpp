#include <c_map_fov.h>

#include <u_pos_iterator.h>
#include <u_pawn.h>


RayData::RayData( int32_t i_iXPos, int32_t i_iYPos )
  : m_cPos( i_iXPos, i_iYPos )
{}


RayCaster::RayCaster( const rumMap* i_pcMap )
  : m_pcMap( i_pcMap )
{}


RayCaster::~RayCaster()
{
  ReleaseRayData();
}


// Initializes the whole process, beginning with an expansion from (0,0).
// Returns a vector of effected coordinates
void RayCaster::CastRays( const rumPosition& i_rcOriginPos, const rumRectangle& i_rcRangeRect )
{
  // debug
  //cout << "Starting to cast rays" << endl;

  ReleaseRayData();

  m_cOriginPos = i_rcOriginPos;
  m_cRangeRect = i_rcRangeRect;

  // Expand the perimeter from the origin
  RayData* pcSourceRay{ new RayData( 0, 0 ) };
  ExpandPerimeter( pcSourceRay );

  RayData* pcCurrentRay{ nullptr };

  while( !m_cPerimeterQueue.empty() )
  {
    // While there's still more rays to check, read the front value and remove it from the queue
    pcCurrentRay = m_cPerimeterQueue.front();
    m_cPerimeterQueue.pop();

    // The current ray should have all the inputs now, so let's process them
    MergeInputs( pcCurrentRay );
    if( !pcCurrentRay->m_bIgnore )
    {
      ExpandPerimeter( pcCurrentRay );
    }
  }

  // We're done! now we step through the filled map, add all visible coordinates to endlist, and clean up.
  // First let's add the origin and delete the first created ray.
  m_cRayMap[m_cOriginPos] = pcSourceRay;
}


// Takes the source and processes rays in all directions away from the origin.
void RayCaster::ExpandPerimeter( RayData* i_pcSourceRay )
{
  // Right
  if( i_pcSourceRay->m_cPos.m_iX >= 0 )
  {
    ProcessRay( new RayData( i_pcSourceRay->m_cPos.m_iX + 1, i_pcSourceRay->m_cPos.m_iY ), i_pcSourceRay );
  }

  // Left
  if( i_pcSourceRay->m_cPos.m_iX <= 0 )
  {
    ProcessRay( new RayData( i_pcSourceRay->m_cPos.m_iX - 1, i_pcSourceRay->m_cPos.m_iY ), i_pcSourceRay );
  }

  // Down
  if( i_pcSourceRay->m_cPos.m_iY >= 0 )
  {
    ProcessRay( new RayData( i_pcSourceRay->m_cPos.m_iX, i_pcSourceRay->m_cPos.m_iY + 1 ), i_pcSourceRay );
  }

  // Up
  if( i_pcSourceRay->m_cPos.m_iY <= 0 )
  {
    ProcessRay( new RayData( i_pcSourceRay->m_cPos.m_iX, i_pcSourceRay->m_cPos.m_iY - 1 ), i_pcSourceRay );
  }
}


// Checks bounds and range limits, assigns inputs and adds the ray to the perimeter
void RayCaster::ProcessRay( RayData* i_pcNewRay, RayData* i_pcInputRay )
{
  const rumPosition cPos( m_cOriginPos.m_iX + i_pcNewRay->m_cPos.m_iX, m_cOriginPos.m_iY + i_pcNewRay->m_cPos.m_iY );

  if( cPos.m_iX < m_cRangeRect.m_cPoint1.m_iX || cPos.m_iX > m_cRangeRect.m_cPoint2.m_iX ||
      cPos.m_iY < m_cRangeRect.m_cPoint1.m_iY || cPos.m_iY > m_cRangeRect.m_cPoint2.m_iY )
  {
    delete i_pcNewRay;
    i_pcNewRay = nullptr;
    return;
  }

  // Check to see if the new ray is already set up. We look it up with the results map.
  const RayMap::iterator iter( m_cRayMap.find( cPos ) );
  if( iter != m_cRayMap.end() )
  {
    //cout << "Position " << newray->location.x << ", " << newray->location.y << " has already been tested" << endl;

    // If it's already there delete the current one and point to the existing one
    delete i_pcNewRay;
    i_pcNewRay = iter->second;
  }

  // Set correct input for the new array
  if( i_pcNewRay->m_cPos.m_iY == i_pcInputRay->m_cPos.m_iY )
  {
    // If we're moving horizontally, mark the input ray as the x input
    i_pcNewRay->m_XInput = i_pcInputRay;
  }
  else
  {
    // Otherwise, mark it as the y input
    i_pcNewRay->m_YInput = i_pcInputRay;
  }

  // Now add the new ray to the search perimeter and map if we haven't done so yet.
  if( !i_pcNewRay->m_bAdded )
  {
    m_cPerimeterQueue.push( i_pcNewRay );
    i_pcNewRay->m_bAdded = true;
    m_cRayMap[cPos] = i_pcNewRay;
  }
}


void RayCaster::ProcessXInput( RayData* i_pcNewRay, RayData* i_pcXInput ) const
{
  if( !i_pcXInput )
  {
    return;
  }

  // Early out if these conditions are met
  if( ( i_pcXInput->m_cObstructionPos.m_iX == 0 ) && ( i_pcXInput->m_cObstructionPos.m_iY == 0 ) )
  {
    return;
  }

  // Progressive X obscurity
  if( i_pcXInput->m_cErrorPos.m_iX > 0 )
  {
    if( i_pcNewRay->m_cObstructionPos.m_iX == 0 )
    {
      // favouring recessive input angle
      i_pcNewRay->m_cErrorPos.m_iX = ( i_pcXInput->m_cErrorPos.m_iX - i_pcXInput->m_cObstructionPos.m_iY );
      i_pcNewRay->m_cErrorPos.m_iY = ( i_pcXInput->m_cErrorPos.m_iY + i_pcXInput->m_cObstructionPos.m_iY );
      i_pcNewRay->m_cObstructionPos = i_pcXInput->m_cObstructionPos;
    }
  }
  // Recessive Y obscurity
  if( i_pcXInput->m_cErrorPos.m_iY <= 0 )
  {
    if( ( i_pcXInput->m_cObstructionPos.m_iY > 0 ) && ( i_pcXInput->m_cErrorPos.m_iX > 0 ) )
    {
      i_pcNewRay->m_cErrorPos.m_iY = ( i_pcXInput->m_cObstructionPos.m_iY + i_pcXInput->m_cErrorPos.m_iY );
      i_pcNewRay->m_cErrorPos.m_iX = ( i_pcXInput->m_cErrorPos.m_iX - i_pcXInput->m_cObstructionPos.m_iY );
      i_pcNewRay->m_cObstructionPos = i_pcXInput->m_cObstructionPos;
    }
  }
}


// The Y input can provide two main pieces of information:
// 1. Progressive Y obscurity
// 2. Recessive X obscurity
void RayCaster::ProcessYInput( RayData* i_pcNewRay, RayData* i_pcYInput ) const
{
  if( !i_pcYInput )
  {
    return;
  }

  // Early out if these conditions are met
  if( ( i_pcYInput->m_cObstructionPos.m_iX == 0 ) && ( i_pcYInput->m_cObstructionPos.m_iY == 0 ) )
  {
    return;
  }

  // Progressive Y obscurity
  if( i_pcYInput->m_cErrorPos.m_iY > 0 )
  {
    if( i_pcNewRay->m_cObstructionPos.m_iY == 0 )
    {
      // favouring recessive input angle
      i_pcNewRay->m_cErrorPos.m_iY = ( i_pcYInput->m_cErrorPos.m_iY - i_pcYInput->m_cObstructionPos.m_iX );
      i_pcNewRay->m_cErrorPos.m_iX = ( i_pcYInput->m_cErrorPos.m_iX + i_pcYInput->m_cObstructionPos.m_iX );
      i_pcNewRay->m_cObstructionPos = i_pcYInput->m_cObstructionPos;
    }
  }
  // Recessive X obscurity
  if( i_pcYInput->m_cErrorPos.m_iX <= 0 )
  {
    if( ( i_pcYInput->m_cObstructionPos.m_iX > 0 ) && ( i_pcYInput->m_cErrorPos.m_iY > 0 ) )
    {
      i_pcNewRay->m_cErrorPos.m_iX = ( i_pcYInput->m_cObstructionPos.m_iX + i_pcYInput->m_cErrorPos.m_iX );
      i_pcNewRay->m_cErrorPos.m_iY = ( i_pcYInput->m_cErrorPos.m_iY - i_pcYInput->m_cObstructionPos.m_iX );
      i_pcNewRay->m_cObstructionPos = i_pcYInput->m_cObstructionPos;
    }
  }
}


// We have all the inputs now, so let's fill the new ray with the appropriate data.
void RayCaster::MergeInputs( RayData* i_pcNewRay )
{
  //cout << "Testing pos " << newray->location.x << ", " << newray->location.y << endl;

  // # JOHNNY MOD - Moved obscurity check to bottom

  // Process individual input information.
  ProcessXInput( i_pcNewRay, i_pcNewRay->m_XInput );
  ProcessYInput( i_pcNewRay, i_pcNewRay->m_YInput );

  // Culling handled here.
  // If both inputs are null, the point is never checked, so ignorance 
  // is propagated trivially in that case.
  if( !i_pcNewRay->m_XInput )
  {
    // cut point (inside edge)
    if( i_pcNewRay->m_YInput->IsObscured() )
    {
      i_pcNewRay->m_bIgnore = true;
    }
  }
  else if( !i_pcNewRay->m_YInput )
  {
    // cut point (inside edge)
    if( i_pcNewRay->m_XInput->IsObscured() )
    {
      i_pcNewRay->m_bIgnore = true;
    }
  }
  // both y and x inputs are valid
  // cut point (within arc of obscurity)
  else if( i_pcNewRay->m_XInput->IsObscured() && i_pcNewRay->m_YInput->IsObscured() )
  {
    // If both inputs are obscured...
    // # JOHNNY MOD - Added diagonal check
    // First make sure the ray is really obscured by checking the diagonal. If it is, cut it.
    if( i_pcNewRay->m_XInput->m_YInput && i_pcNewRay->m_XInput->m_YInput->IsObscured() )
    {
      //cout << "Position " << newray->location.x << "," << newray->location.y << " is obscured" << endl;
      i_pcNewRay->m_bIgnore = true;
    }
    // # END JOHNNY MOD
  }

  // # JOHNNY MOD - Moved from above and added "obscurer" feature
  if( !i_pcNewRay->m_bIgnore )
  {
    // If the point is not cut, check for an obstacle and update the data
    const rumPosition cPos( m_cOriginPos.m_iX + i_pcNewRay->m_cPos.m_iX, m_cOriginPos.m_iY + i_pcNewRay->m_cPos.m_iY );

    // See if this particular square obscures vision
    if( m_pcMap->PositionBlocksSight( cPos ) )
    {
      // Determine if the obscurer is already obscured
      if( ( i_pcNewRay->m_cObstructionPos.m_iX == 0 ) && ( i_pcNewRay->m_cObstructionPos.m_iY == 0 ) )
      {
        // No previous obstruction, so this obstruction is visible
        i_pcNewRay->m_bObscurer = true;
      }
      else
      {
        // Previous obstruction, if the error matches the obstruction, this obstruction is already obscured
        if( i_pcNewRay->m_cErrorPos.m_iX != i_pcNewRay->m_cObstructionPos.m_iX &&
            i_pcNewRay->m_cErrorPos.m_iY != i_pcNewRay->m_cObstructionPos.m_iY )
        {
          // This obstruction is visible
          i_pcNewRay->m_bObscurer = true;
        }
      }

      i_pcNewRay->m_cObstructionPos.m_iX = abs( i_pcNewRay->m_cPos.m_iX );
      i_pcNewRay->m_cObstructionPos.m_iY = abs( i_pcNewRay->m_cPos.m_iY );
      i_pcNewRay->m_cErrorPos = i_pcNewRay->m_cObstructionPos;
    }
  }
  // # END JOHNNY MOD
}


void RayCaster::ReleaseRayData()
{
  for( auto& iter : m_cRayMap )
  {
    // Delete the raydata
    delete iter.second;
  }

  // Swap queue for an empty one
  std::queue<RayData*> qEmpty;
  std::swap( m_cPerimeterQueue, qEmpty );

  m_cRayMap.clear();
}
