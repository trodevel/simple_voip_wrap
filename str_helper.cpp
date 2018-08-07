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

#include "str_helper.h"                 // self

#include <typeinfo>

#include "simple_voip/str_helper.h"     // simple_voip::StrHelper::write()

namespace simple_voip_wrap {

std::ostream & StrHelper::write( std::ostream & os, const simple_voip::IObject & o )
{
    if( typeid( o ) == typeid( simple_voip::wrap::PlayFileRequest ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::PlayFileRequest&>( o );

        os << " " << m.req_id << " " << m.call_id << " " << m.filename;
    }
    else if( typeid( o ) == typeid( simple_voip::wrap::PlayFileResponse ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::PlayFileResponse&>( o );

        os << " " << m.req_id;
    }
    else if( typeid( o ) == typeid( simple_voip::wrap::PlayFileErrorResponse ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::PlayFileErrorResponse&>( o );

        os << " " << m.req_id << " " << m.errorcode << " " << m.descr;
    }
    else if( typeid( o ) == typeid( simple_voip::wrap::PlayFileStopped ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::PlayFileStopped&>( o );

        os << " " << m.req_id;
    }
    else if( typeid( o ) == typeid( simple_voip::wrap::RecordFileRequest ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::RecordFileRequest&>( o );

        os << " " << m.req_id << " " << m.call_id << " " << m.filename;
    }
    else if( typeid( o ) == typeid( simple_voip::wrap::RecordFileResponse ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::RecordFileResponse&>( o );

        os << " " << m.req_id;
    }
    else if( typeid( o ) == typeid( simple_voip::wrap::RecordFileErrorResponse ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::RecordFileErrorResponse&>( o );

        os << " " << m.req_id << " " << m.errorcode << " " << m.descr;
    }
    else if( typeid( o ) == typeid( simple_voip::wrap::RecordFileStopped ) )
    {
        os << typeid( o ).name();

        auto & m = dynamic_cast<const simple_voip::wrap::RecordFileStopped&>( o );

        os << " " << m.req_id;
    }
    else
    {
        return simple_voip::StrHelper::write( os, o );
    }

    return os;
}

const std::string StrHelper::to_string( const simple_voip::IObject & o )
{
    std::stringstream os;

    write( os, o );

    return os.str();
}

} // namespace simple_voip_wrap
