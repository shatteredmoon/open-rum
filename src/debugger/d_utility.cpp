/*

Squirrel ImGui Debugger Utility

MIT License

Copyright 2022 Jonathon Blake Wood-Brooks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <d_utility.h>

#include <d_settings.h>

#include <algorithm>
#include <vector>


namespace rumDebugUtility
{
  std::string BuildInstanceDescription( HSQUIRRELVM i_pcVM, bool i_bValuesAsHex )
  {
#if DEBUG_OUTPUT
    SQInteger iTopBegin{ sq_gettop( i_pcVM ) };
#endif

    HSQOBJECT sqInstance;
    sq_getstackobj( i_pcVM, -1, &sqInstance );
    if( !sq_isinstance( sqInstance ) )
    {
#if DEBUG_OUTPUT
      SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
      assert( iTopBegin == iTopEnd );
#endif

      return "";
    }

    std::string strDesc;

    // Get the class of the instance so that we can iterate over its member keys
    sq_getclass( i_pcVM, -1 );

    HSQOBJECT sqClass;
    sq_getstackobj( i_pcVM, -1, &sqClass );

    std::string strName{ GetObjectName( i_pcVM, sqClass ) };
    if( !strName.empty() )
    {
      strDesc = '<' + strName + ">\n";
    }

    bool bSuccess{ true };
    SQInteger iIndex{ 0 };
    do
    {
      // Push iteration index
      sq_pushinteger( i_pcVM, iIndex );

      // Get the next member
      bSuccess = SQ_SUCCEEDED( sq_next( i_pcVM, -2 ) );
      if( bSuccess )
      {
        if( 0 != iIndex )
        {
          strDesc += "\n";
        }

        // Get the member value
        HSQOBJECT sqValue;
        sq_getstackobj( i_pcVM, -1, &sqValue );

        // Get the member key
        HSQOBJECT sqKey;
        sq_getstackobj( i_pcVM, -2, &sqKey );

        // Get the next iteration index
        sq_getinteger( i_pcVM, -3, &iIndex );

        sq_pop( i_pcVM, 3 );

        // Put the instance and the desired key on the stack
        sq_pushobject( i_pcVM, sqInstance );
        sq_pushstring( i_pcVM, sq_objtostring( &sqKey ), -1 );

        strDesc += FormatVariable( i_pcVM, -1, i_bValuesAsHex );

        // Get the object from the table
        HSQOBJECT slotObj;
        sq_get( i_pcVM, -2 );
        sq_getstackobj( i_pcVM, -1, &slotObj );

        std::string strValue{ FormatVariable( i_pcVM, -1, i_bValuesAsHex ) };

        // Pop the fetched object
        sq_poptop( i_pcVM );

        // Copy the key name
        strDesc += ": " + strValue;
      }

      // Pop the iterator index
      sq_poptop( i_pcVM );
    } while( bSuccess );

    // Pop the fetched class
    sq_poptop( i_pcVM );

#if DEBUG_OUTPUT
    SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    assert( iTopBegin == iTopEnd );
#endif

    return strDesc;
  }


  std::string BuildTableDescription( HSQUIRRELVM i_pcVM, bool i_bValuesAsHex )
  {
    std::string strDesc;

#if DEBUG_OUTPUT
    SQInteger iTopBegin{ sq_gettop( i_pcVM ) };
#endif

    HSQOBJECT sqObject;
    sq_getstackobj( i_pcVM, -1, &sqObject );
    std::string strName{ GetObjectName( i_pcVM, sqObject ) };
    if( !strName.empty() )
    {
      strDesc = '<' + strName + ">\n";
    }

    // For now, skip over large tables
    const auto iSize{ sq_getsize( i_pcVM, -1 ) };
    if( iSize < 100 )
    {
      using TableEntry = std::pair<std::string, SQInteger>;
      std::vector<TableEntry> vTableEntries;

      SQInteger iIndex{ 0 };
      sq_pushinteger( i_pcVM, iIndex );

      for( SQInteger i = 0;
           SQ_SUCCEEDED( sq_getinteger( i_pcVM, -1, &iIndex ) ) && SQ_SUCCEEDED( sq_next( i_pcVM, -2 ) );
           ++i )
      {
        sq_poptop( i_pcVM );
        vTableEntries.emplace_back( TableEntry{ FormatVariable( i_pcVM, -1, i_bValuesAsHex ), iIndex } );
        sq_poptop( i_pcVM );
      }

      sq_poptop( i_pcVM );

      /*std::sort(vTableEntries.begin(), vTableEntries.end(),
                 []( const TableEntry& lhs, const TableEntry& rhs ) -> bool
                 {
                   return lhs.first < rhs.first;
                 } );*/

      int32_t iCount{ 0 };
      for( const auto& iter : vTableEntries )
      {
        sq_pushinteger( i_pcVM, iter.second );
        if( !SQ_SUCCEEDED( sq_next( i_pcVM, -2 ) ) )
        {
          sq_poptop( i_pcVM );
          break;
        }

        const auto strValue{ FormatVariable( i_pcVM, -1, i_bValuesAsHex ) };
        if( strValue.empty() )
        {
          sq_pop( i_pcVM, 2 );
        }
        else
        {
          if( 0 != iCount )
          {
            strDesc += "\n";
          }

          sq_poptop( i_pcVM );
          strDesc += FormatVariable( i_pcVM, -1, i_bValuesAsHex ) + ": " + strValue;
          sq_poptop( i_pcVM );
        }

        sq_poptop( i_pcVM );

        ++iCount;
      }
    }
    else
    {
      strDesc += "<table too large>";
    }

#if DEBUG_OUTPUT
    SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    assert( iTopBegin == iTopEnd );
#endif

    return strDesc;
  }


  SQObject FindSymbol( HSQUIRRELVM i_pcVM, const std::string& i_strVariable, uint32_t i_iLocalStackLevel )
  {
    SQObject sqObject{};

    SQInteger iTop{ sq_gettop( i_pcVM ) };

    sq_pushroottable( i_pcVM );

    // First, check to see if this is a class/instance variable with a member
    size_t szDotPosition{ i_strVariable.find( '.' ) };
    if( szDotPosition != std::string::npos )
    {
      std::string strClass{ i_strVariable.substr( 0, szDotPosition ) };
      std::string strMember{ i_strVariable.substr( szDotPosition + 1 ) };

      // Fetch the class/instance from the root table
      sq_pushstring( i_pcVM, _SC( strClass.c_str() ), -1 );
      if( SQ_SUCCEEDED( sq_get( i_pcVM, -2 ) ) )
      {
        sq_getstackobj( i_pcVM, -1, &sqObject );

        // Fetch the class/instance member
        sq_pushstring( i_pcVM, _SC( strMember.c_str() ), -1 );
        if( SQ_SUCCEEDED( sq_get( i_pcVM, -2 ) ) )
        {
          sq_getstackobj( i_pcVM, -1, &sqObject );
        }
      }
      else
      {
        // Not found in the root table, so check locals
        int32_t iIndex{ 0 };
        const SQChar* strName{ sq_getlocal( i_pcVM, i_iLocalStackLevel, iIndex++ ) };
        while( strName )
        {
          SQObjectType eType{ sq_gettype( i_pcVM, -1 ) };
          if( ( ( eType == OT_CLASS ) || ( eType == OT_INSTANCE ) ) && ( strClass.compare( strName ) == 0 ) )
          {
            // A matching class/instance was found, so try to fetch the member
            sq_pushstring( i_pcVM, _SC( strMember.c_str() ), -1 );
            if( SQ_SUCCEEDED( sq_get( i_pcVM, -2 ) ) )
            {
              sq_getstackobj( i_pcVM, -1, &sqObject );
              sq_poptop( i_pcVM );
            }

            sq_poptop( i_pcVM );
          }

          sq_poptop( i_pcVM );

          strName = sq_getlocal( i_pcVM, i_iLocalStackLevel, iIndex++ );
        }
      }
    }
    else
    {
      // Check the root table
      sq_pushstring( i_pcVM, _SC( i_strVariable.c_str() ), -1 );
      if( SQ_SUCCEEDED( sq_get( i_pcVM, -2 ) ) )
      {
        sq_getstackobj( i_pcVM, -1, &sqObject );
      }
      else
      {
        // Fetch the const table
        sq_pushconsttable( i_pcVM );
        sq_getstackobj( i_pcVM, -1, &sqObject );
        sq_pop( i_pcVM, 1 );

        // Push the const table and desired string
        sq_pushobject( i_pcVM, sqObject );
        sq_pushstring( i_pcVM, _SC( i_strVariable.c_str() ), -1 );

        // Fetch the result
        sq_get( i_pcVM, -2 );
        sq_getstackobj( i_pcVM, -1, &sqObject );
        sq_pop( i_pcVM, 2 );
      }
    }

    sq_settop( i_pcVM, iTop );

    return sqObject;
  }


  std::string FormatVariable( HSQUIRRELVM i_pcVM, const SQInteger i_iIndex, bool i_bValuesAsHex )
  {
    std::string strVariable;

#if DEBUG_OUTPUT
    SQInteger iTopBegin{ sq_gettop( i_pcVM ) };
#endif

    const auto eType{ sq_gettype( i_pcVM, i_iIndex ) };
    switch( eType )
    {
      case OT_BOOL:
      {
        SQBool b;
        sq_getbool( i_pcVM, i_iIndex, &b );
        strVariable = b ? "true" : "false";
        break;
      }

      case OT_ARRAY:
      case OT_CLASS:
      case OT_TABLE:
        strVariable = BuildTableDescription( i_pcVM, i_bValuesAsHex );
        break;

      case OT_INSTANCE:
        strVariable = BuildInstanceDescription( i_pcVM, i_bValuesAsHex );
        break;

      case OT_CLOSURE:
      {
        if( SQ_SUCCEEDED( sq_getclosurename( i_pcVM, i_iIndex ) ) )
        {
          const ::SQChar* strVal = nullptr;
          if( SQ_SUCCEEDED( sq_getstring( i_pcVM, i_iIndex, &strVal ) ) )
          {
            strVariable = strVal ? strVal : "<anonymous closure>";
            sq_poptop( i_pcVM );
          }
        }
        else
        {
          strVariable = "<invalid closure>";
        }

        SQUnsignedInteger iParams{ 0 };
        SQUnsignedInteger iFreeVars{ 0 };
        if( SQ_SUCCEEDED( sq_getclosureinfo( i_pcVM, i_iIndex, &iParams, &iFreeVars ) ) )
        {
          strVariable += "(" + std::to_string( iParams ) + " params)";
        }
        break;
      }

      case OT_FLOAT:
      {
        SQFloat f;
        sq_getfloat( i_pcVM, i_iIndex, &f );
        strVariable = std::to_string( f );
        break;
      }

      case OT_INTEGER:
      {
        SQInteger i;
        sq_getinteger( i_pcVM, i_iIndex, &i );
        if( i_bValuesAsHex )
        {
          static char strHighBuffer[20] = { '0', 'x', '\0' };
          static char strLowBuffer[20] = { '\0' };

#if _WIN64 || __x86_64__ || __ppc64__
          if( sizeof( SQInteger ) == sizeof( int64_t ) && ( i >> 32 ) > 0 )
          {
            _itoa_s( static_cast<int32_t>( i >> 32 ), &strHighBuffer[2], 18, 16 );
            strVariable = strHighBuffer;
            strVariable += _itoa_s( static_cast<int32_t>( i ), &strLowBuffer[0], 20, 16 );
          }
          else
          {
            _itoa_s( static_cast<int32_t>( i ), &strHighBuffer[2], 18, 16 );
            strVariable = strHighBuffer;
          }
#else
          _itoa_s( static_cast<int32_t>( i ), &strHighBuffer[2], 18, 16 );
          strVariable = strHighBuffer;
#endif
        }
        else
        {
          strVariable = std::to_string( i );
        }
        break;
      }

      case OT_NULL:
        strVariable = "null";
        break;

      case OT_STRING:
      {
        const SQChar* s;
        sq_getstring( i_pcVM, i_iIndex, &s );
        strVariable = s;
        break;
      }

      default:
        strVariable = '<' + GetTypeName( eType ) + '>';
        break;
    }

#if DEBUG_OUTPUT
    SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    assert( iTopBegin == iTopEnd );
#endif

    return strVariable;
  }


  std::string FormatVariable( HSQUIRRELVM i_pcVM, HSQOBJECT i_sqObject, bool i_bValuesAsHex )
  {
#if DEBUG_OUTPUT
    SQInteger iTopBegin{ sq_gettop( i_pcVM ) };
#endif

    sq_pushobject( i_pcVM, i_sqObject );
    std::string strVariable{ FormatVariable( i_pcVM, -1, i_bValuesAsHex ) };
    sq_poptop( i_pcVM );

#if DEBUG_OUTPUT
    SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    assert( iTopBegin == iTopEnd );
#endif

    return strVariable;
  }


  std::string GetObjectName( HSQUIRRELVM i_pcVM, HSQOBJECT i_sqObject )
  {
    // TODO - the hash value for found entries could be cached, but performance isn't really a requirement here

    if( !( sq_isarray( i_sqObject ) || sq_istable( i_sqObject ) || sq_isclass( i_sqObject ) ||
           sq_isinstance( i_sqObject ) ) )
    {
      return "";
    }

#if DEBUG_OUTPUT
    SQInteger iTop{ sq_gettop( i_pcVM ) };
#endif

    std::string strName;

    sq_pushobject( i_pcVM, i_sqObject );
    const auto iObjectHash{ sq_gethash( i_pcVM, -1 ) };
    sq_poptop( i_pcVM );

    sq_pushroottable( i_pcVM );
    const auto iRootTableHash{ sq_gethash( i_pcVM, -1 ) };

    // Early out if this is the root table
    if( iRootTableHash == iObjectHash )
    {
      sq_poptop( i_pcVM );

#if DEBUG_OUTPUT
      SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
      assert( iTop == iTopEnd );
#endif

      return "<RootTable>";
    }

    // Push iteration index
    SQInteger iIndex{ 0 };

    bool bSuccess{ false };

    do
    {
      sq_pushinteger( i_pcVM, iIndex );

      // Iterate
      bSuccess = SQ_SUCCEEDED( sq_next( i_pcVM, -2 ) );
      if( bSuccess )
      {
        // The object value is at position -1, so get its hash
        const auto iIterHash{ sq_gethash( i_pcVM, -1 ) };
        if( iObjectHash == iIterHash )
        {
          // Get the entry's key
          HSQOBJECT sqKey;
          sq_getstackobj( i_pcVM, -2, &sqKey );
          strName = sq_objtostring( &sqKey );
        }

        // Next iteration index
        sq_getinteger( i_pcVM, -3, &iIndex );

        // Pop the key and value
        sq_pop( i_pcVM, 2 );
      }

      // Pop the iterator index
      sq_poptop( i_pcVM );
    } while( bSuccess && strName.empty() );

    // Pop the root table
    sq_poptop( i_pcVM );

#if DEBUG_OUTPUT
    SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    assert( iTop == iTopEnd );
#endif

    return strName;
  }


  // Note that Squirrel's IdType2Name helper could be used here, but it requires an include of cassert which would blow
  // away any cost savings and it isn't as precise as the below solution since it doesn't differentiate between certain
  // things like a closure and a nativeclosure.
  std::string GetTypeName( SQObjectType i_eObjectType )
  {
    std::string strType;

    switch( i_eObjectType )
    {
      case OT_ARRAY:         strType = "array";         break;
      case OT_BOOL:          strType = "bool";          break;
      case OT_CLASS:         strType = "class";         break;
      case OT_CLOSURE:       strType = "closure";       break;
      case OT_FLOAT:         strType = "float";         break;
      case OT_GENERATOR:     strType = "generator";     break;
      case OT_INSTANCE:      strType = "instance";      break;
      case OT_INTEGER:       strType = "integer";       break;
      case OT_NATIVECLOSURE: strType = "nativeclosure"; break;
      case OT_NULL:          strType = "null";          break;
      case OT_OUTER:         strType = "outer";         break;
      case OT_STRING:        strType = "string";        break;
      case OT_TABLE:         strType = "table";         break;
      case OT_THREAD:        strType = "thread";        break;
      case OT_USERDATA:      strType = "userdata";      break;
      case OT_USERPOINTER:   strType = "userpointer";   break;
      case OT_WEAKREF:       strType = "weakref";       break;
      default:               strType = "unknown";       break;
    }

    return strType;
  }


  bool IsOperator( const std::string& i_strToken )
  {
    static std::vector<std::string> vOperators
    {
      "<-", "~", "!", "typeof", "++", "--", "/", "*", "%", "+", "-", "<<", ">>", ">>>", "<", "<=", ">", ">=", "==",
      "!=", "<=>", "&", "^", "|", "&&", "in", "||", "?", ":", "+=", "=", "-=", ","
    };

    auto iter{ std::find_if( vOperators.begin(), vOperators.end(),
                             [&i_strToken]( const std::string& strToken )
      {
        return strToken.compare( i_strToken ) == 0;
      } ) };

    return iter != vOperators.end();
  }


  bool IsReservedWord( const std::string& i_strToken )
  {
    static std::vector<std::string> vReservedWords
    {
      "base", "break", "case", "catch", "class", "clone", "continue", "const", "default", "delete", "else", "enum",
      "extends", "for", "foreach", "function", "if", "in", "local", "null", "resume", "return", "switch", "this",
      "throw", "try", "typeof", "while", "yield", "constructor", "instanceof", "true", "false", "static"
    };

    auto iter{ std::find_if( vReservedWords.begin(), vReservedWords.end(),
                             [&i_strToken]( const std::string& strToken )
      {
        return strToken.compare( i_strToken ) == 0;
      } ) };

    return iter != vReservedWords.end();
  }


  bool IsUnknownType( SQObjectType i_eObjectType )
  {
    auto eRawType{ _RAW_TYPE( i_eObjectType ) };
    return( eRawType < OT_NULL && eRawType > _RT_OUTER );
  }
}
