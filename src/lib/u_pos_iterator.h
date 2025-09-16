#ifndef _U_POS_ITERATOR_H_
#define _U_POS_ITERATOR_H_

#include <u_map.h>
#include <u_pawn.h>

class rumPositionIterator
{
public:

  rumPositionIterator( rumMap::PositionData* i_pcPosition );

  // Get the first object in line, regardless of type
  Sqrat::Object& GetFirstObject()
  {
    Reset();
    return GetNextObject();
  }

  // Get the first object in line by ptr, regardless of type
  rumPawn* GetFirstObjectPtr()
  {
    Reset();
    return GetNextObjectPtr();
  }

  // Get the first object matching the specified class
  Sqrat::Object& GetFirst( rumPawn::PawnType i_ePawnType )
  {
    Reset();
    return GetNext( i_ePawnType );
  }

  // Get the first object matching the specified class
  rumPawn* GetFirstPtr( rumPawn::PawnType i_ePawnType )
  {
    Reset();
    return GetNextPtr( i_ePawnType );
  }

  // Get the next object in line, regardless of type
  Sqrat::Object& GetNextObject();

  // Get the next object ptr in line, regardless of type
  rumPawn* GetNextObjectPtr();

  // Get the next object matching the specified class
  Sqrat::Object& GetNext( rumPawn::PawnType i_ePawnType );

  // Get the next object matching the specified class and asset type
  Sqrat::Object& GetNext( rumPawn::PawnType i_ePawnType, rumAssetID i_eAssetID );

  // Get the next object matching the specified class
  rumPawn* GetNextPtr( rumPawn::PawnType i_ePawnType );

  int32_t GetNumObjects() const
  {
    return (int32_t)( m_pcPosition ? m_pcPosition->m_cPawnDataList.size() : 0 );
  }

  rumAssetID GetTileID()
  {
    return m_pcPosition ? m_pcPosition->m_eTileID : INVALID_ASSET_ID;
  }

  // Effectively ends iteration by moving iterator to the end of the list
  void Stop()
  {
    m_iter = m_end;
  }

  // Start iteration over at the beginning
  void Reset()
  {
    if( m_pcPosition )
    {
      m_iter = m_pcPosition->m_cPawnDataList.begin();
    }
  }

  // Tests to see if the iterator is at the end of the list
  bool Done()
  {
    return ( !m_pcPosition || ( m_iter == m_end ) );
  }

  static void ScriptBind();

private:

  rumMap::PositionData* m_pcPosition{ nullptr };
  rumMap::PawnDataList::iterator m_iter;
  rumMap::PawnDataList::iterator m_end;

  Sqrat::Object m_sqInstance;
};

#endif // _U_CELL_ITERATOR_H_
