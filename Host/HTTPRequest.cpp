#include "stdafx.h"
#include "HTTPRequest.h"

CHTTPRequest::CHTTPRequest()
{
	m_pHeaders.CoCreateInstance( OLESTR("Scripting.Dictionary") );
}
CHTTPRequest::~CHTTPRequest()
{
}
BOOL CHTTPRequest::IsMPFD()
{
	_variant_t Content_Type;
	if( SUCCEEDED( m_pHeaders->get_Item( &_variant_t( _bstr_t( "Content-Type" ) ), &Content_Type ) ) )
		return (BOOL) wcsstr( Content_Type.bstrVal, L"multipart/form-data" );
	else
		return false;
}
void CHTTPRequest::Parse( BYTE* buf )
{
	USES_CONVERSION;
	BYTE line[8192];
	char method[8192];
	char URI[8192];

	VARIANT name, value;
	name.vt = value.vt = VT_BSTR;

	DWORD i;

	for( i = 1; buf[i-1] != '\r' && buf[i] != '\n'; line[i-2] = buf[++i - 2] );
	line[i-1] = 0;
	buf = buf + i + 1;
	sscanf( (char*)line, "%s%s", method, URI );
	StringCchCopyW( m_URI, 8192, CA2W( URI ) );

	while( buf[0] != '\r' && buf[1] != '\n' )
	{
        for( i = 1; buf[i-1] != '\r' && buf[i] != '\n'; line[i-2] = buf[++i - 2] );
		line[i-1] = 0;
		buf = buf + i + 1;
		if( strstr( (char*) line, ": " ) )
		{
			strstr( (char*) line, ": " )[0] = 0;
			name.bstrVal  = SysAllocString( A2OLE( (char*) line ) );
			value.bstrVal = SysAllocString( A2OLE( (char*) line + strlen( (char*) line ) + 2 ) );
			m_pHeaders->Add( &name, &value );
			SysFreeString( name.bstrVal  );
			SysFreeString( value.bstrVal );
		}
	}

	if( strstr( method, "GET" ) )
		m_method = HTTP_METHOD_GET;
	else if( strstr(  method, "PUT" ) )
		m_method = HTTP_METHOD_PUT;
	else if( strstr( method, "POST" ) && IsMPFD() )
		m_method = HTTP_METHOD_MPFDPOST;
	else if( strstr( method, "POST" ) )
		m_method = HTTP_METHOD_POST;
	else
		m_method = HTTP_METHOD_UNKNOWN;
}
