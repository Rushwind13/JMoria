// Shim to provide missing GoogleTest symbols due to ABI mismatch
// and custom printers for JMoria types
#include <JMDefs.h>
#include <JVector.h>
#include <ostream>

namespace testing
{

// Custom printer for TVector2 types (prevents GoogleTest from using byte-level printing)
namespace internal
{
template <typename T> void PrintTo( const TVector2<T> &vec, std::ostream *os )
{
    *os << "TVector2(" << vec.x << ", " << vec.y << ")";
}

// Explicit instantiations for the types we use
template void PrintTo( const TVector2<int> &vec, std::ostream *os );
template void PrintTo( const TVector2<float> &vec, std::ostream *os );
} // namespace internal

// Provide the missing symbol that GoogleTest's internal2 namespace expects
namespace internal2
{
void PrintBytesInObjectTo( const unsigned char *bytes, unsigned long count, std::ostream *os )
{
    *os << "(" << count << " bytes)";
}
} // namespace internal2

} // namespace testing
