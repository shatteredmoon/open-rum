#include <u_property_container.h>

#include <u_property_asset.h>
#include <u_utility.h>


Sqrat::Object rumPropertyContainer::AdjustProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqDelta )
{
  rumAssert( INVALID_ASSET_ID != i_ePropertyID );
  if( INVALID_ASSET_ID == i_ePropertyID )
  {
    return Sqrat::Object();
  }

  rumPropertyAsset* pcAsset{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return Sqrat::Object();
  }

  if( pcAsset->GetValueType() != PropertyValueType::Bitfield &&
      pcAsset->GetValueType() != PropertyValueType::Integer &&
      pcAsset->GetValueType() != PropertyValueType::Float )
  {
    rumAssertMsg( false, "Error: Adjust called on non-numeric property type. Prefer SetProperty() instead." );
    SetProperty( i_ePropertyID, i_sqDelta );
    return i_sqDelta;
  }

  Sqrat::Object sqMax{ pcAsset->GetMaxValue() };
  Sqrat::Object sqExisting{ GetProperty( i_ePropertyID ) };
  Sqrat::Object sqValue;

  if( ( pcAsset->GetValueType() == PropertyValueType::Integer ) ||
      ( pcAsset->GetValueType() == PropertyValueType::Bitfield ) )
  {
    rumScript::SetValue( sqValue, sqExisting.Cast<uint64_t>() + i_sqDelta.Cast<uint64_t>() );
  }
  else
  {
    rumScript::SetValue( sqValue, sqExisting.Cast<float>() + i_sqDelta.Cast<float>() );
  }

  if( SetProperty( i_ePropertyID, sqValue ) == RESULT_SUCCESS )
  {
    return sqValue;
  }

  return sqExisting;
}


void rumPropertyContainer::AllocatePropertyBuckets( uint32_t i_uiNumRows )
{
  const uint32_t uiNeededSize{ (uint32_t)m_hashProperties.size() + i_uiNumRows };
  m_hashProperties.reserve( uiNeededSize );
}


Sqrat::Object rumPropertyContainer::CapProperty( rumAssetID i_ePropertyID )
{
  rumAssert( INVALID_ASSET_ID != i_ePropertyID );
  if( INVALID_ASSET_ID == i_ePropertyID )
  {
    return Sqrat::Object();
  }

  rumPropertyAsset* pcAsset{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return Sqrat::Object();
  }

  if( pcAsset->GetValueType() != PropertyValueType::Bitfield &&
      pcAsset->GetValueType() != PropertyValueType::Integer &&
      pcAsset->GetValueType() != PropertyValueType::Float )
  {
    rumAssertMsg( false, "Error: Can't cap a property value for non-numeric types" );
    return Sqrat::Object();
  }

  Sqrat::Object sqMax{ pcAsset->GetMaxValue() };
  Sqrat::Object sqExisting{ GetProperty( i_ePropertyID ) };

  // Set to cap
  if( SetProperty( i_ePropertyID, sqMax ) == RESULT_SUCCESS )
  {
    return sqMax;
  }

  return sqExisting;
}


Sqrat::Object rumPropertyContainer::GetProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqDefaultValue )
{
  const auto& iter{ m_hashProperties.find( i_ePropertyID ) };
  return iter != m_hashProperties.end() ? iter->second : i_sqDefaultValue;
}


bool rumPropertyContainer::HasProperty( rumAssetID i_ePropertyID )
{
  const auto& iter{ m_hashProperties.find( i_ePropertyID ) };
  return iter != m_hashProperties.end();
}


void rumPropertyContainer::RemoveAllProperties()
{
  for( auto iter : m_hashProperties )
  {
    RemoveProperty( iter.first );
  }
}


void rumPropertyContainer::RemoveProperty( rumAssetID i_ePropertyID )
{
#if PROPERTY_DEBUG
  std::string strInfo{ "Object [" };
  strInfo += rumStringUtils::ToHexString64( GetGameID() );
  strInfo += "] removed ";
  strInfo += rumPropertyAsset::Fetch( i_ePropertyID )->GetName();
  strInfo += " (";
  strInfo += rumStringUtils::ToHexString( i_ePropertyID );
  strInfo += ")";
  Logger::LogStandard( strInfo );
#endif // PROPERTY_DEBUG

  if( m_hashProperties.erase( i_ePropertyID ) != 0 )
  {
    OnPropertyRemoved( i_ePropertyID );
  }
}


// static
void rumPropertyContainer::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumPropertyContainer, Sqrat::NoConstructor<rumPropertyContainer> > cPropertyContainer( pcVM, "rumPropertyContainer" );
  cPropertyContainer
    .Func( "AdjustProperty", &AdjustProperty )
    .Func( "CapProperty", &CapProperty )
    .Func( "HasProperty", &HasProperty )
    .Func( "RemoveAllProperties", &RemoveAllProperties )
    .Func( "RemoveProperty", &RemoveProperty )
    .Func( "SetProperty", &SetProperty )
    .Overload<Sqrat::Object( rumPropertyContainer::* )( rumAssetID, Sqrat::Object )>( "GetProperty", &GetProperty )
    .Overload<Sqrat::Object( rumPropertyContainer::* )( rumAssetID )>( "GetProperty", &GetProperty );
  Sqrat::RootTable( pcVM ).Bind( "rumPropertyContainer", cPropertyContainer );
}


// virtual
int32_t rumPropertyContainer::Serialize( rumResource& io_rcResource )
{
  int32_t eResult{ RESULT_SUCCESS };

  if( io_rcResource.IsSaving() )
  {
    // Build a temporary hash of persistent properties
    PropertyContainer m_hashPersistentProperties;
    for( const auto& iter : m_hashProperties )
    {
      const rumAssetID eAssetID{ iter.first };
      const rumPropertyAsset* pcAsset{ rumPropertyAsset::Fetch( eAssetID ) };
      rumAssert( pcAsset );
      if( !pcAsset || !pcAsset->IsPersistent() )
      {
        continue;
      }

      m_hashPersistentProperties.insert( std::make_pair( iter.first, iter.second ) );
    }

    // Write out the number of properties that will be serialized
    io_rcResource << (rumDWord)m_hashPersistentProperties.size();

    // Write all persistent properties
    for( const auto& iter : m_hashPersistentProperties )
    {
      const rumAssetID eAssetID{ iter.first };
      const rumPropertyAsset* pcAsset{ rumPropertyAsset::Fetch( eAssetID ) };
      rumAssert( pcAsset );

      RUM_COUT_IFDEF( PROPERTY_DEBUG,
                      "Serializing property " << pcAsset->GetName() << " [" <<
                      rumStringUtils::ToHexString( eAssetID ) << "\n" );

      Sqrat::Object sqValue{ iter.second };

      io_rcResource << (rumDWord)pcAsset->GetAssetID();
      io_rcResource << (rumDWord)pcAsset->GetValueType();

      if( pcAsset->IsAssetRef() ||
          ( pcAsset->GetValueType() == PropertyValueType::Bitfield ) ||
          ( pcAsset->GetValueType() == PropertyValueType::Integer )  ||
          ( pcAsset->GetValueType() == PropertyValueType::StringToken ) )
      {
        io_rcResource << (rumQWord)sqValue.Cast<uint64_t>();
      }
      else if( pcAsset->GetValueType() == PropertyValueType::Float )
      {
        io_rcResource << (rumDWord)sqValue.Cast<float>();
      }
      else if( pcAsset->GetValueType() == PropertyValueType::Bool )
      {
        io_rcResource << (rumByte)sqValue.Cast<bool>();
      }
      else if( pcAsset->GetValueType() == PropertyValueType::String )
      {
        const std::string strValue{ sqValue.Cast<std::string>() };
        io_rcResource << (rumDWord)strValue.length();
        io_rcResource << strValue;
      }
      else
      {
        rumAssertMsg( false, "Unsupported value type" );
        return RESULT_FAILED;
      }
    }
  }
  else if( io_rcResource.IsLoading() && !io_rcResource.IsEndOfFile() )
  {
    static_assert( sizeof( rumAssetID ) == sizeof( rumDWord ) );

    rumDWord iNumProperties{ 0 };
    io_rcResource << iNumProperties;

    for( rumDWord i = 0; i < iNumProperties && !io_rcResource.IsEndOfFile(); ++i )
    {
      rumAssetID ePropertyID{ INVALID_ASSET_ID };
      io_rcResource << ePropertyID;

      const rumPropertyAsset* pcAsset{ rumPropertyAsset::Fetch( ePropertyID ) };
      rumAssert( pcAsset );
      if( !pcAsset )
      {
        continue;
      }

      rumDWord iValueType{ 0 };
      io_rcResource << iValueType;
      PropertyValueType eValueType{ PropertyValueType( iValueType ) };

      // TODO - Need to convert the old value to the new type
      rumAssert( pcAsset->GetValueType() == eValueType );

      Sqrat::Object sqValue;

      if( pcAsset->IsAssetRef() ||
          ( pcAsset->GetValueType() == PropertyValueType::Bitfield ) ||
          ( pcAsset->GetValueType() == PropertyValueType::Integer )  ||
          ( pcAsset->GetValueType() == PropertyValueType::StringToken ) )
      {
        rumQWord qValue{ 0 };
        io_rcResource << qValue;
        rumScript::SetValue( sqValue, qValue );
      }
      else if( pcAsset->GetValueType() == PropertyValueType::Float )
      {
        assert( sizeof( float ) == sizeof( rumDWord ) );

        rumDWord iValue{ 0 };
        io_rcResource << iValue;
        const float fValue{ static_cast<float>( iValue ) };
        rumScript::SetValue( sqValue, fValue );
      }
      else if( pcAsset->GetValueType() == PropertyValueType::Bool )
      {
        rumByte iValue{ 0 };
        io_rcResource << iValue;
        const bool bValue{ iValue ? true : false };
        rumScript::SetValue( sqValue, bValue );
      }
      else if( pcAsset->GetValueType() == PropertyValueType::String )
      {
        rumAssertMsg( false, "Untested" );

        rumDWord iLength{ 0 };
        io_rcResource << iLength;
        std::string strValue;
        strValue.resize( iLength );
        io_rcResource << strValue;
      }
      else
      {
        rumAssertMsg( false, "Unsupported value type" );
        return RESULT_FAILED;
      }

      SetProperty( ePropertyID, sqValue );
    }
  }

  return RESULT_SUCCESS;
}


int32_t rumPropertyContainer::SetProperty( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue )
{
#if PROPERTY_DEBUG
  std::string strInfo{ "Object [" };
  strInfo += rumStringUtils::ToHexString64( GetGameID() );
  strInfo += "] set ";
  strInfo += rumPropertyAsset::Fetch( i_ePropertyID )->GetName();
  strInfo += " (";
  strInfo += rumStringUtils::ToHexString( i_ePropertyID );
  strInfo += ") value ";
  strInfo += ::ToString( i_sqValue );
  Logger::LogStandard( strInfo );
#endif // PROPERTY_DEBUG

  if( !rumPropertyAsset::Validate( i_ePropertyID, i_sqValue ) )
  {
    return RESULT_FAILED;
  }

  int32_t eResult{ RESULT_SUCCESS };

  const auto& iter( m_hashProperties.find( i_ePropertyID ) );
  if( iter == m_hashProperties.end() )
  {
    // The property doesn't exist, insert a new one
    const auto cPair{ m_hashProperties.insert( std::make_pair( i_ePropertyID, i_sqValue ) ) };
    if( cPair.second )
    {
      OnPropertyUpdated( i_ePropertyID, i_sqValue, true /* property added */ );
    }
    else
    {
      std::string strInfo{ "Error: Failed to set property (" };
      strInfo += rumStringUtils::ToHexString( i_ePropertyID );
      strInfo += ")";
      Logger::LogStandard( strInfo );

      eResult = RESULT_FAILED;
    }
  }
  else if( !( iter->second.GetObjectA()._unVal.raw == i_sqValue.GetObjectA()._unVal.raw ) )
  {
    // The property already exists, but does not match the currently stored value
    iter->second = i_sqValue;
    OnPropertyUpdated( i_ePropertyID, i_sqValue, false /* property updated */ );
  }

  return eResult;
}


std::string rumPropertyContainer::ToString() const
{
  if( m_hashProperties.empty() )
  {
    return rumStringUtils::NullString();
  }

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  std::string strDesc{ "Instance Properties:\n\n" };

  for( const auto& iter : m_hashProperties )
  {
    const rumAssetID ePropertyID{ iter.first };
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
    rumAssert( pcProperty );
    if( pcProperty )
    {
      strDesc += "  " + pcProperty->GetName() + " = ";

      std::string strValue;
      if( pcProperty->IsAssetRef() )
      {
        strValue = rumStringUtils::ToHexString( iter.second.Cast<rumAssetID>() );
      }
      else
      {
        strValue = rumScript::ToString( iter.second );
      }

      const auto cPair{ rumScript::EvalOptionalFunc( Sqrat::RootTable( pcVM ), "OnPropertyDesc", std::string{},
                                                     pcProperty, iter.second ) };
      if( !cPair.second.empty() )
      {
        strDesc += cPair.second + " (" + strValue + ") ";
      }
      else
      {
        strDesc += strValue + " ";
      }

      strDesc += "<" + rumScript::GetObjectTypeName( iter.second ) + ">\n";
    }
  }

  return strDesc;
}
