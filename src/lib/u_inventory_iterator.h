#ifndef _U_INVENTORY_ITERATOR_H_
#define _U_INVENTORY_ITERATOR_H_

#include <u_pawn.h>
#include <u_structs.h>

class rumInventory;


class rumInventoryIterator
{
public:

  rumInventoryIterator( InventoryContainer* i_pContainer );

  // Get the first object in line, regardless of type
  Sqrat::Object& GetFirstObject()
  {
    Reset();
    return GetNextObject();
  }

  // Get the first object in line by ptr, regardless of type
  rumInventory* GetFirstObjectPtr()
  {
    Reset();
    return GetNextObjectPtr();
  }

  // Get the first object matching the specified property type and value
  Sqrat::Object& GetFirst( rumAssetID i_eAssetID )
  {
    Reset();
    return GetNext( i_eAssetID );
  }

  // Get the first object matching the specified class
  rumInventory* GetFirstPtr( rumAssetID i_eAssetID )
  {
    Reset();
    return GetNextPtr( i_eAssetID );
  }

  // Get the next object in line, regardless of type
  Sqrat::Object& GetNextObject();

  // Get the next object ptr in line, regardless of type
  rumInventory* GetNextObjectPtr();

  // Get the next object matching the specified class
  Sqrat::Object& GetNext( rumAssetID i_eAssetID );

  // Get the next object matching the specified class
  rumInventory* GetNextPtr( rumAssetID i_eAssetID );

  uint32_t GetNumObjects() const
  {
    return (uint32_t)( m_pContainer ? m_pContainer->size() : 0 );
  }

  // Effectively ends iteration by moving iterator to the end of the list
  void Stop()
  {
    m_iterCurrent = m_iterEnd;
  }

  // Start iteration over at the beginning
  void Reset()
  {
    if( m_pContainer )
    {
      m_iterCurrent = m_pContainer->begin();
    }
  }

  // Tests to see if the iterator is at the end of the list
  bool Done()
  {
    return ( !m_pContainer || ( m_iterCurrent == m_iterEnd ) );
  }

  static void ScriptBind();

private:

  InventoryContainer* m_pContainer{ nullptr };
  InventoryContainer::iterator m_iterCurrent;
  InventoryContainer::iterator m_iterEnd;

  Sqrat::Object m_sqInstance;
};

#endif // _U_INVENTORY_ITERATOR_H_
