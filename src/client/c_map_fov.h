#ifndef _C_MAP_FOV_H_
#define _C_MAP_FOV_H_

/*
This is a bastardized version of the code found at these locations in the order listed:

1. The original java version of the code (not sure who to credit)
   http://www.geocities.com/temerra/los_rays.html
2. Most of the equivalent c++ source was taken from a post by someone by the name of BirdOfPrey on www.forumwarz.com
   http://www.forumwarz.com/discussions/view_post/235663
3. Nick Wiggill of Visual Harmonics provided a Flash version which proved to be very handy for visualizing the
   algorithm and for finding bugs in the original source
   http://www.visualharmonics.co.uk/actionscript-3/as3-conversion-of-modelling-rays-for-line-of-sight-in-an-object-rich-world/#comment-233

CALCULATING FIELD OF VISION
These classes will attempt to make use of the method described here: http://www.geocities.com/temerra/los_rays.html
With minor adjustments for diagonals. It will return an array of coordinates of tiles that are in the FoV of the given
source point within the given range in the given map.
*/

#include <u_map.h>
#include <u_structs.h>

#include <map>
#include <queue>


template <class T>
struct CoordLess
{
  bool operator() ( const T& i_iPos1, const T& i_iPos2 ) const
  {
    if( i_iPos1.m_iX < i_iPos2.m_iX )
    {
      return true;
    }
    else if( i_iPos1.m_iX == i_iPos2.m_iX )
    {
      return i_iPos1.m_iY < i_iPos2.m_iY;
    }

    return false;
  }
};

typedef CoordLess<rumPoint> CoordCompare;


// Contains the info for each ray, and a function for determining its obscurity
struct RayData
{
  RayData( int32_t i_iXPos, int32_t i_iYPos );

  // Returns true if the ray is obscured
  bool IsObscured() const
  {
    return( ( m_cErrorPos.m_iX > 0 && m_cErrorPos.m_iX <= m_cObstructionPos.m_iX ) ||
            ( m_cErrorPos.m_iY > 0 && m_cErrorPos.m_iY <= m_cObstructionPos.m_iY ) );
  }

  bool IsVisible() const
  {
    return ( m_bObscurer || !( m_bIgnore || IsObscured() ) );
  }

  // Location relative to origin
  rumPoint m_cPos;

  // Location of saved obstruction data
  rumPoint m_cObstructionPos;

  rumPoint m_cErrorPos;

  // The parent inputs
  RayData* m_XInput{ nullptr };
  RayData* m_YInput{ nullptr };

  // True if the ray was already added
  bool m_bAdded{ false };

  // True if this ray has been culled
  bool m_bIgnore{ false };

  // True if this ray is an obstruction
  bool m_bObscurer{ false };
};


typedef std::map<rumPosition, RayData*, CoordCompare> RayMap;

// RayCaster class - includes all the functions for determining a FoV.
// Perhaps later RayCaster can be adapted to carry rays of effects as well as light and vision.
class RayCaster
{
public:

  RayCaster( const rumMap* i_pcMap );
  ~RayCaster();

  // Initializes ray caster, starting the search with (0,0)
  void CastRays( const rumPosition& i_rcOriginPos, const rumRectangle& i_rcRangeRect );

  const RayMap& GetRayMap() const
  {
    return m_cRayMap;
  }

private:

  // Expands outwards and away from the origin
  void ExpandPerimeter( RayData* i_pcSourceRay );

  // Checks bounds and range, assigns inputs, adds ray to perimeter if valid
  void ProcessRay( RayData* i_pcNewRay, RayData* i_pcInputRay );

  // Populates the new ray with the correct data
  void MergeInputs( RayData* i_pcNewRay );
  void ProcessXInput( RayData* i_pcNewRay, RayData* i_pcXInput ) const;
  void ProcessYInput( RayData* i_pcNewRay, RayData* i_pcYInput ) const;

  void ReleaseRayData();

  // For determining where obstacles are
  const rumMap* m_pcMap{ nullptr };

  // The source of the rays
  rumPosition m_cOriginPos;

  // Range of the effect
  rumRectangle m_cRangeRect;

  // The queue of rays to check
  std::queue<RayData*> m_cPerimeterQueue;

  // Key-value map of coordinates to Ray info
  RayMap m_cRayMap;
};

#endif // _C_MAP_FOV_H_
