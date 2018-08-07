/*

Simple VOIP Wrap. Get Duration.

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

// $Revision: 9614 $ $Date:: 2018-08-07 #$ $Author: serge $

#ifndef SIMPLE_VOIP_WRAP__I_GET_DURATION_H
#define SIMPLE_VOIP_WRAP__I_GET_DURATION_H

#include <string>

namespace simple_voip_wrap {

class IGetDuration
{
public:
    virtual ~IGetDuration() {};

    virtual double get_duration( const std::string & filename ) = 0;
};

} // namespace simple_voip_wrap

#endif  // SIMPLE_VOIP_WRAP__I_GET_DURATION_H
