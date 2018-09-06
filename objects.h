/*

Simple VOIP Wrap objects.

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

// $Revision: 9706 $ $Date:: 2018-09-05 #$ $Author: serge $

#ifndef SIMPLE_VOIP_WRAP__OBJECTS_H
#define SIMPLE_VOIP_WRAP__OBJECTS_H

#include "simple_voip/objects.h"    // Object...

namespace simple_voip {

namespace wrap {

// ******************* IN-CALL REQUESTS *******************

struct PlayFileRequest: public simple_voip::PlayFileRequest
{
};

struct PlayFileResponse: public simple_voip::PlayFileResponse
{
};

struct PlayFileErrorResponse: public simple_voip::ErrorResponse
{
};

struct PlayFileStopped: public CallbackObject
{
    uint32_t    call_id;
    uint32_t    req_id;
};

struct RecordFileRequest: public simple_voip::RecordFileRequest
{
    double  duration;
};

struct RecordFileResponse: public simple_voip::RecordFileResponse
{
};

struct RecordFileErrorResponse: public simple_voip::ErrorResponse
{
};

struct RecordFileStopped: public CallbackObject
{
    uint32_t    call_id;
    uint32_t    req_id;
};

} // namespace wrap
} // namespace simple_voip

#endif  // SIMPLE_VOIP_WRAP__OBJECTS_H
