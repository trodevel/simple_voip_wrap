/*

String Helper. Provides to_string() function.

Copyright (C) 2018 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 9612 $ $Date:: 2018-08-07 #$ $Author: serge $

#ifndef SIMPLE_VOIP_WRAP__STR_HELPER_H
#define SIMPLE_VOIP_WRAP__STR_HELPER_H

#include <sstream>          // std::ostringstream

#include "objects.h"        // PlayFileRequest

namespace simple_voip_wrap {

class StrHelper
{
public:
    static std::ostream & write( std::ostream & os, const simple_voip::IObject & l );
    static const std::string to_string( const simple_voip::IObject & o );
};

} // namespace simple_voip_wrap

#endif // SIMPLE_VOIP_WRAP__STR_HELPER_H
