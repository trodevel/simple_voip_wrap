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

// $Revision: 9747 $ $Date:: 2018-09-21 #$ $Author: serge $

#ifndef SIMPLE_VOIP_WRAP__OBJECT_FACTORY_H
#define SIMPLE_VOIP_WRAP__OBJECT_FACTORY_H

#include "objects.h"    // Object...

#include "simple_voip/object_factory.h"     // init_req_id

namespace simple_voip {
namespace wrap {

inline PlayFileRequest *create_PlayFileRequest( uint32_t req_id, uint32_t call_id, const std::string & filename )
{
    auto * res = new PlayFileRequest;

    init_req_id( res, req_id );

    res->call_id    = call_id;
    res->filename   = filename;

    return res;
}

inline PlayFileStopped *create_PlayFileStopped( uint32_t call_id, uint32_t req_id, uint32_t errorcode, const std::string & error_msg )
{
    auto * res = new PlayFileStopped;

    init_req_id( res, req_id );

    res->call_id    = call_id;
    res->errorcode  = errorcode;
    res->error_msg  = error_msg;

    return res;
}

inline RecordFileRequest *create_RecordFileRequest( uint32_t req_id, uint32_t call_id, const std::string & filename, double duration )
{
    auto * res = new RecordFileRequest;

    init_req_id( res, req_id );

    res->call_id    = call_id;
    res->filename   = filename;
    res->duration   = duration;

    return res;
}

inline RecordFileStopped *create_RecordFileStopped( uint32_t call_id, uint32_t req_id, uint32_t errorcode, const std::string & error_msg )
{
    auto * res = new RecordFileStopped;

    init_req_id( res, req_id );

    res->call_id    = call_id;
    res->errorcode  = errorcode;
    res->error_msg  = error_msg;

    return res;
}

} // namespace wrap
} // namespace simple_voip

#endif  // SIMPLE_VOIP_WRAP__OBJECT_FACTORY_H
