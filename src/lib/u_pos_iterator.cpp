#include <u_pos_iterator.h>


rumPositionIterator::rumPositionIterator( rumMap::PositionData* i_pcPosition )
  : m_pcPosition( i_pcPosition )
{
  if( i_pcPosition )
  {
    Reset();
    m_end = m_pcPosition->m_cPawnDataList.end();
  }
}


Sqrat::Object& rumPositionIterator::GetNextObject()
{
  m_sqInstance.Release();

  if( !Done() )
  {
    rumGameObject* pcObject{ rumGameObject::Fetch( m_iter->m_iPawnID ) };
    if( pcObject )
    {
      m_sqInstance = pcObject->GetScriptInstance();
    }

    ++m_iter;
  }

  return m_sqInstance;
}


rumPawn* rumPositionIterator::GetNextObjectPtr()
{
  rumPawn* pcPawn{ nullptr };

  m_sqInstance.Release();

  if( !Done() )
  {
    pcPawn = rumPawn::Fetch( m_iter->m_iPawnID );
    if( pcPawn )
    {
      m_sqInstance = pcPawn->GetScriptInstance();
    }

    ++m_iter;
  }

  return pcPawn;
}


Sqrat::Object& rumPositionIterator::GetNext( rumPawn::PawnType i_ePawnType )
{
  if( !Done() )
  {
    bool bMatch{ false };

    // Keep iterating until a matching class is encountered or the end of the list is reached
    do
    {
      const rumPawn* pcPawn{ GetNextPtr( i_ePawnType ) };
      if( pcPawn )
      {
        bMatch = ( pcPawn->GetPawnType() == i_ePawnType );
      }
    } while( !Done() && !bMatch );

    // If we encountered the end of the list, make absolutely sure the last object encountered was a good match.
    // Otherwise, we reset it so that it is not returned.
    if( Done() && !bMatch )
    {
      m_sqInstance.Release();
    }
  }
  else
  {
    m_sqInstance.Release();
  }

  return m_sqInstance;
}


Sqrat::Object& rumPositionIterator::GetNext( rumPawn::PawnType i_ePawnType, rumAssetID i_eAssetID )
{
  bool bMatch{ false };

  if( !Done() )
  {
    // Keep iterating until a matching class is encountered or the end of the list is reached
    do
    {
      const rumPawn* pcPawn{ GetNextPtr( i_ePawnType ) };
      if( pcPawn )
      {
        bMatch = ( pcPawn->GetPawnType() == i_ePawnType ) && ( pcPawn->GetAssetID() == i_eAssetID );
      }
    } while( !Done() && !bMatch );
  }

  // If we encountered the end of the list, make absolutely sure the last object encountered was a good match.
  // Otherwise, we reset it so that it is not returned.
  if( Done() && !bMatch )
  {
    m_sqInstance.Release();
  }

  return m_sqInstance;
}


rumPawn* rumPositionIterator::GetNextPtr( rumPawn::PawnType i_ePawnType )
{
  rumPawn* pcPawn{ nullptr };
  bool bMatch{ false };

  if( !Done() )
  {
    // Keep iterating until a matching type is encountered or the end of the list is reached
    do
    {
      pcPawn = GetNextObjectPtr();
      if( pcPawn )
      {
        bMatch = ( pcPawn->GetPawnType() == i_ePawnType );
      }
    } while( !Done() && !bMatch );
  }

  // If we encountered the end of the list, make absolutely sure the last object encountered was a good match.
  // Otherwise, we reset it so that it is not returned.
  if( Done() && !bMatch )
  {
    m_sqInstance.Release();
    pcPawn = nullptr;
  }

  if( pcPawn )
  {
    m_sqInstance = pcPawn->GetScriptInstance();
  }

  return pcPawn;
}


void rumPositionIterator::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumPositionIterator> cPositionIterator( pcVM, "rumPositionIterator" );
  cPositionIterator
    .Func( "GetFirstObject", &GetFirstObject )
    .Func( "GetFirst", &GetFirst )
    .Func( "GetNextObject", &GetNextObject )
    .Func( "GetNumObjects", &GetNumObjects )
    .Func( "GetTileID", &GetTileID )
    .Func( "Reset", &Reset )
    .Func( "Stop", &Stop )
    .Func( "Stopped", &Done )
    .Overload< Sqrat::Object& ( rumPositionIterator::* )( rumPawn::PawnType, rumAssetID )>( "GetNext", &GetNext )
    .Overload< Sqrat::Object& ( rumPositionIterator::* )( rumPawn::PawnType )>( "GetNext", &GetNext );
  Sqrat::RootTable( pcVM ).Bind( "rumPositionIterator", cPositionIterator );
}
