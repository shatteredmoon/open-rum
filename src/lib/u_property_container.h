#pragma once

#include <u_enum.h>
#include <u_script.h>


// An object for holding properties. This class is meant to be inherited from and extended.
class rumPropertyContainer
{
public:

  using PropertyContainer = std::unordered_map< rumAssetID, Sqrat::Object >;

  // If the type is numeric, modifies the existing value with the delta and returns the new value, otherwise simply
  // sets the property's value to the delta.
  Sqrat::Object AdjustProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqDelta );

  // Allocate buckets efficiently based on the expected number of property database row fetches
  void AllocatePropertyBuckets( uint32_t i_uiNumRows );

  // If the type is numeric, bumps the existing value to the max value the type can store and returns the new value,
  // otherwise returns null.
  Sqrat::Object CapProperty( rumAssetID i_ePropertyID );

  const PropertyContainer& GetProperties() const
  {
    return m_hashProperties;
  }

  virtual Sqrat::Object GetProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqDefaultValue );
  Sqrat::Object GetProperty( rumAssetID i_ePropertyID )
  {
    return GetProperty( i_ePropertyID, Sqrat::Object() );
  }

  const PropertyContainer& GetInstanceProperties() const
  {
    return m_hashProperties;
  }

  virtual bool HasProperty( rumAssetID i_ePropertyID );

  void RemoveProperty( rumAssetID i_ePropertyID );
  void RemoveAllProperties();

  int32_t SetProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue );

  virtual std::string ToString() const;

  static void ScriptBind();

protected:

  virtual void OnPropertyRemoved( rumAssetID i_ePropertyID ) = 0;
  virtual void OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded ) = 0;

  virtual int32_t Serialize( rumResource& io_rcResource );

protected:

  // Property data hash
  PropertyContainer m_hashProperties;
};
