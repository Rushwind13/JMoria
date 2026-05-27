#include "JMDefs.h"

static const int MAX_STRING = 256;
class JString
{
public:
    JString() { SetValue( "" ); }
    JString( const char *szValue ) { SetValue( szValue ); }
    ~JString() {}

    const char *GetValue() const { return m_szValue; }
    void SetValue( const char *szValue )
    {
        memset( m_szValue, 0, MAX_STRING );
        Util::jstrcpy( m_szValue, szValue );
    }
    operator const char *() const { return m_szValue; }
    const char *operator=( const char *szValue )
    {
        SetValue( szValue );
        return m_szValue;
    }
    bool operator==( const char *szValue ) const
    {
        return Util::jstrcmp( m_szValue, szValue ) == 0;
    }
    bool operator!=( const char *szValue ) const
    {
        return Util::jstrcmp( m_szValue, szValue ) != 0;
    }
    const char *operator>>( const char *szValue )
    {
        SetValue( szValue );
        return m_szValue;
    }

protected:
private:
    char m_szValue[MAX_STRING];
};