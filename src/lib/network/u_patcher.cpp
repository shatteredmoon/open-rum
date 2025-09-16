#include <network/u_patcher.h>

#include <u_db.h>
#include <u_log.h>
#include <u_rum.h>
#include <u_utility.h>

#include <filesystem>
#include <fstream>

#define PATCH_NAME "patch"

std::vector<std::string> rumPatcher::s_cPatchCategoriesVector = { "Standard", "Conditional", "Remove", "Ignore" };
rumPatcher::PatchMap rumPatcher::s_cPatchEntries;


// static
void rumPatcher::ExportCSVFiles( const std::string& i_strPath )
{
  const std::filesystem::path fsBasePath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / PATCH_NAME );

  for( int32_t iIndex = 0; iIndex < rumUtility::ToUnderlyingType( PatchType::NumTypes ); ++iIndex )
  {
    const auto ePatchType{ static_cast<PatchType>( iIndex ) };

    std::string strCategory{ s_cPatchCategoriesVector[iIndex] };
    rumStringUtils::ToLower( strCategory );

    std::filesystem::path fsFilePath( fsBasePath );
    fsFilePath += "_" + strCategory + CSV_EXTENSION;

    ExportCSVFile( ePatchType, fsFilePath.string() );
  }
}


// static
void rumPatcher::ExportCSVFile( rumPatcher::PatchType i_ePatchType, const std::string& i_strFilePath )
{
  std::filesystem::path fsFilePath{ i_strFilePath };
  std::filesystem::create_directories( fsFilePath.parent_path() );

  std::ofstream cOutfile( fsFilePath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() )
  {
    const auto iter{ s_cPatchEntries.find( i_ePatchType ) };
    if( iter != s_cPatchEntries.end() )
    {
      auto& rcOutputList{ iter->second };
      rcOutputList.sort();

      for( const auto& rcPatchFileInfo : rcOutputList )
      {
        cOutfile << rcPatchFileInfo.m_strFile;

        if( rumPatcher::PatchType::Standard == i_ePatchType )
        {
          // Standard files additionally write out the checksum and filesize
          cOutfile << ',' << rcPatchFileInfo.m_strFileCRC << ',' << rcPatchFileInfo.m_uiFileSize << '\n';
        }
        else
        {
          cOutfile << '\n';
        }
      }
    }

    cOutfile.close();

    // Remove the file stub if nothing was written
    if( std::filesystem::file_size( fsFilePath ) == 0U )
    {
      std::filesystem::remove( fsFilePath );
    }
  }
}


// static
void rumPatcher::ExportDBTable( const PatchType i_ePatchType )
{
  std::string strCategory{ GetTypeName( i_ePatchType ) };
  rumStringUtils::ToLower( strCategory );

  rumDatabase::Query( rumDatabase::Patch_DatabaseID, "DROP TABLE " + strCategory );

  std::string strQuery{ "CREATE TABLE [" + strCategory + "]([filepath] TEXT NOT NULL UNIQUE" };

  if( i_ePatchType != PatchType::Remove )
  {
    strQuery += ",[crc] TEXT NOT NULL,[size] INTEGER NOT NULL);";
  }
  else
  {
    strQuery += ");";
  }

  rumDatabase::Query( rumDatabase::Patch_DatabaseID, strQuery );

  strQuery = "BEGIN;";

  const auto iter{ s_cPatchEntries.find( i_ePatchType ) };
  if( iter != s_cPatchEntries.end() )
  {
    auto& rcOutputList{ iter->second };
    rcOutputList.sort();

    for( const auto& rcPatchFileInfo : rcOutputList )
    {
      if( i_ePatchType != PatchType::Remove )
      {
        strQuery += "INSERT INTO " + strCategory + " (filepath,crc,size) VALUES ('";
        strQuery += rcPatchFileInfo.m_strFile;
        strQuery += "','";
        strQuery += rcPatchFileInfo.m_strFileCRC;
        strQuery += "',";
        strQuery += rumStringUtils::ToString64( rcPatchFileInfo.m_uiFileSize );
        strQuery += ");";
      }
      else
      {
        strQuery += "INSERT INTO " + strCategory + " (filepath) VALUES ('";
        strQuery += rcPatchFileInfo.m_strFile;
        strQuery += "');";
      }
    }
  }

  strQuery += "END;";

  rumDatabase::Query( rumDatabase::Patch_DatabaseID, strQuery );
}


// static
void rumPatcher::ExportDBTables( std::vector<PatchType>&& i_cPatchTypesVector )
{
  for( int32_t iIndex = 0; iIndex < rumUtility::ToUnderlyingType( PatchType::NumTypes ); ++iIndex )
  {
    const auto iter{ std::find( i_cPatchTypesVector.begin(), i_cPatchTypesVector.end(), static_cast<PatchType>( iIndex ) ) };
    if( iter != i_cPatchTypesVector.end() )
    {
      ExportDBTable( *iter );
    }
  }

  rumDatabase::Query( rumDatabase::Patch_DatabaseID, "VACUUM" );
}


// static
const std::string_view rumPatcher::GetPatchDatabaseName()
{
  return PATCH_NAME ".db";
}


const std::string_view rumPatcher::GetPatchFileName()
{
  return PATCH_NAME;
}


const std::string_view rumPatcher::GetPatchTableName()
{
  return PATCH_NAME;
}


// static
const std::string_view rumPatcher::GetTypeName( PatchType i_ePatchType )
{
  return s_cPatchCategoriesVector.at( rumUtility::ToUnderlyingType( i_ePatchType ) );
}


// static
void rumPatcher::ImportCSVFile( PatchType i_ePatchType, const std::string& i_strFilePath )
{
  s_cPatchEntries[i_ePatchType].clear();

  std::filesystem::path fsFilePath{ i_strFilePath };

  std::ifstream cFile( fsFilePath.c_str(), std::ios::in );
  if( cFile.is_open() )
  {
    std::string strRow;
    enum { COL_FILE, COL_HASH, COL_SIZE };

    while( !cFile.eof() )
    {
      std::getline( cFile, strRow );
      if( !strRow.empty() )
      {
        const auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };

        rumPatcher::rumPatchFileInfo cPatchFileInfo;
        cPatchFileInfo.m_strFile = vFields.at( COL_FILE ).c_str();

        if( rumPatcher::PatchType::Standard == i_ePatchType )
        {
          // Standard patch files also have a checksum and file size
          cPatchFileInfo.m_strFileCRC = vFields.at( COL_HASH ).c_str();
          cPatchFileInfo.m_uiFileSize = rumStringUtils::ToUInt( vFields.at( COL_SIZE ) );
        }

        s_cPatchEntries[i_ePatchType].push_back( std::move( cPatchFileInfo ) );
      }
    }
  }
}


// static
void rumPatcher::ImportDBTable( PatchType i_ePatchType )
{
  std::string strCategory{ GetTypeName( i_ePatchType ) };
  rumStringUtils::ToLower( strCategory );

  std::string strQuery{ "SELECT filepath" };

  if( i_ePatchType != PatchType::Remove )
  {
    strQuery += ",crc,size";
  }

  strQuery += " FROM " + strCategory;

  const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Patch_DatabaseID, strQuery ) };
  if( pcQuery && !pcQuery->IsError() )
  {
    if( i_ePatchType != PatchType::Remove )
    {
      enum{ COL_FILEPATH, COL_CRC, COL_SIZE };

      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        rumPatchFileInfo cPatchFileInfo;

        cPatchFileInfo.m_strFile = pcQuery->FetchString( i, COL_FILEPATH );
        cPatchFileInfo.m_strFileCRC = pcQuery->FetchString( i, COL_CRC );
        cPatchFileInfo.m_uiFileSize = pcQuery->FetchInt64( i, COL_SIZE );

        s_cPatchEntries[i_ePatchType].push_back( cPatchFileInfo );
      }
    }
    else
    {
      enum{ COL_FILEPATH };

      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        rumPatchFileInfo cPatchFileInfo;

        cPatchFileInfo.m_strFile = pcQuery->FetchString( i, COL_FILEPATH );

        s_cPatchEntries[i_ePatchType].push_back( cPatchFileInfo );
      }
    }
  }
}


// static
void rumPatcher::Init( const std::string& i_strPath )
{
  LoadPatchData( i_strPath );
}


// static
void rumPatcher::LoadPatchData( const std::string& i_strPath )
{
  const std::filesystem::path fsBasePath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / PATCH_NAME );

  for( int32_t iIndex = 0; iIndex < rumUtility::ToUnderlyingType( PatchType::NumTypes ); ++iIndex )
  {
    const auto ePatchType{ static_cast<PatchType>( iIndex ) };

    std::string strCategory{ s_cPatchCategoriesVector[iIndex] };
    rumStringUtils::ToLower( strCategory );

    // Try CSV files first
    std::filesystem::path fsFilePath( fsBasePath );
    fsFilePath += "_" + strCategory + CSV_EXTENSION;

    if( std::filesystem::exists( fsFilePath ) )
    {
      ImportCSVFile( ePatchType, fsFilePath.string() );
    }
    else if( ePatchType != PatchType::Ignore )
    {
      ImportDBTable( ePatchType );
    }
  }
}


// static
void rumPatcher::Shutdown()
{
  s_cPatchEntries.clear();
}
