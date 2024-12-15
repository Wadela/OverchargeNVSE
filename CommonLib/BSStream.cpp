#include "BSStream.hpp"

BSStream* BSStream::Create(BSStream* apThis) {
    return ThisStdCall<BSStream*>(0x43CFD0, apThis);
}