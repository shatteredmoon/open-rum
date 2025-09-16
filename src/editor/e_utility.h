#pragma once

#include <u_enum.h>

class QComboBox;


namespace rumControlUtils
{
  void BuildComboFromAssetType( QComboBox& io_rcCombo, PropertyValueType i_eValueType );
  void BuildComboFromEnumType( QComboBox& io_rcCombo, std::string_view i_strEnum );
  void BuildComboFromStringTable( QComboBox& io_rcCombo, std::string_view i_strStringTable );

  void UpdateEditableState( QTableWidgetItem& io_rcCell, PropertyValueType i_eValueType );
}

namespace rumValueUtils
{
  void ConvertPropertyValue( Sqrat::Object& sqValue, PropertyValueType i_eExistingType, PropertyValueType i_eNewType );
  rumAssetID GetFirstAsset( PropertyValueType i_eAssetType );
}
