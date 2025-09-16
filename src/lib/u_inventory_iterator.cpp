#include <u_inventory_iterator.h>

#include <u_inventory.h>


rumInventoryIterator::rumInventoryIterator( InventoryContainer* i_pContainer )
  : m_pContainer( i_pContainer )
{
  if( m_pContainer )
  {
    Reset();
    m_iterEnd = m_pContainer->end();
  }
}


Sqrat::Object& rumInventoryIterator::GetNextObject()
{
  m_sqInstance.Release();

  if( !Done() )
  {
    rumGameObject* pcObject{ rumGameObject::Fetch( *m_iterCurrent ) };
    if( pcObject )
    {
      m_sqInstance = pcObject->GetScriptInstance();
    }

    ++m_iterCurrent;
  }

  return m_sqInstance;
}


rumInventory* rumInventoryIterator::GetNextObjectPtr()
{
  rumInventory* pcInventory{ nullptr };

  m_sqInstance.Release();

  if( !Done() )
  {
    pcInventory = rumInventory::Fetch( *m_iterCurrent );
    if( pcInventory )
    {
      m_sqInstance = pcInventory->GetScriptInstance();
    }

    ++m_iterCurrent;
  }

  return pcInventory;
}


Sqrat::Object& rumInventoryIterator::GetNext( rumAssetID i_eAssetID )
{
  if( !Done() )
  {
    bool bMatch{ false };

    // Keep iterating until a matching class is encountered or the end of the list is reached
    do
    {
      rumInventory* pcInventory{ GetNextPtr( i_eAssetID ) };
      if( pcInventory )
      {
        bMatch = ( pcInventory->GetAssetID() == i_eAssetID );
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


rumInventory* rumInventoryIterator::GetNextPtr( rumAssetID i_eAssetID )
{
  rumInventory* pcInventory{ nullptr };
  bool bMatch{ false };

  if( !Done() )
  {
    // Keep iterating until a matching type is encountered or the end of the list is reached
    do
    {
      pcInventory = GetNextObjectPtr();
      if( pcInventory )
      {
        bMatch = ( pcInventory->GetAssetID() == i_eAssetID );
      }
    } while( !Done() && !bMatch );
  }

  // If we encountered the end of the list, make absolutely sure the last object encountered was a good match.
  // Otherwise, we reset it so that it is not returned.
  if( Done() && !bMatch )
  {
    m_sqInstance.Release();
    pcInventory = nullptr;
  }

  if( pcInventory )
  {
    m_sqInstance = pcInventory->GetScriptInstance();
  }

  return pcInventory;
}


void rumInventoryIterator::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumInventoryIterator> cInventoryIterator( pcVM, "rumInventoryIterator" );
  cInventoryIterator
    .Func( "GetFirstObject", &GetFirstObject )
    .Func( "GetFirst", &GetFirst )
    .Func( "GetNextObject", &GetNextObject )
    .Func( "GetNext", &GetNext )
    .Func( "GetNumObjects", &GetNumObjects )
    .Func( "Reset", &Reset )
    .Func( "Stop", &Stop )
    .Func( "Stopped", &Done );
  Sqrat::RootTable( pcVM ).Bind( "rumInventoryIterator", cInventoryIterator );
}
