/*

Example.

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

// $Revision: 9640 $ $Date:: 2018-08-08 #$ $Author: serge $

#include <iostream>         // cout
#include <typeinfo>
#include <sstream>          // stringstream
#include <atomic>           // std::atomic
#include <vector>           // std::vector

#include "simple_voip/objects.h"
#include "simple_voip/str_helper.h"
#include "simple_voip/object_factory.h"         // simple_voip::create_initiate_call_request
#include "simple_voip/i_simple_voip_callback.h" // simple_voip::ISimpleVoipCallback

#include "simple_voip_dummy/dummy.h"            // simple_voip_dummy::Dummy
#include "simple_voip_dummy/init_config.h"      // simple_voip_dummy::init_config

#include "utils/dummy_logger.h"                 // dummy_log_set_log_level
#include "utils/request_id_gen.h"               // RequestIdGen
#include "scheduler/scheduler.h"                // Scheduler
#include "wav_tools/get_wav_duration.h"         // get_wav_duration()

#include "wrap.h"                               // simple_voip_wrap::Wrap
#include "object_factory.h"                     // simple_voip::wrap::create_PlayFileRequest
#include "i_get_duration.h"                     // IGetDuration

struct DurationGetter: virtual public simple_voip_wrap::IGetDuration
{
    virtual ~DurationGetter() {}

    double get_duration( const std::string & filename ) override
    {
        return wav_tools::get_wav_duration( filename );
    }
};

class Callback: virtual public simple_voip::ISimpleVoipCallback
{
public:
    Callback():
        voips_( nullptr ),
        req_id_gen_( nullptr )
    {
    }

    void init( simple_voip::ISimpleVoip * wrap, utils::IRequestIdGen * req_id_gen )
    {
        voips_      = wrap;
        req_id_gen_ = req_id_gen;
    }

    // interface ISimpleVoipCallback
    void consume( const simple_voip::CallbackObject * req )
    {
        std::cout << "got " << *req << std::endl;

        delete req;
    }

    void control_thread()
    {
        std::cout << "type exit or quit to quit: " << std::endl;
        std::cout << "call <party>" << std::endl;
        std::cout << "drop <call_id>" << std::endl;
        std::cout << "play <call_id> <file>" << std::endl;
        std::cout << "rec <call_id> <file> <duration>" << std::endl;

        std::string input;

        while( true )
        {
            std::cout << "enter command: ";

            std::getline( std::cin, input );

            std::cout << "your input: " << input << std::endl;

            bool b = process_input( input );

            if( b == false )
                break;
        };

        std::cout << "exiting ..." << std::endl;
    }

private:
    bool process_input( const std::string & input )
    {
        std::string cmd;

        std::stringstream stream( input );
        if( stream >> cmd )
        {
            if( cmd == "exit" || cmd == "quit" )
            {
                return false;
            }
            else if( cmd == "call" )
            {
                auto req_id = req_id_gen_->get_next_request_id();

                std::string s;
                stream >> s;

                voips_->consume( simple_voip::create_initiate_call_request( req_id, s ) );
            }
            else if( cmd == "drop" )
            {
                auto req_id = req_id_gen_->get_next_request_id();

                uint32_t call_id;
                stream >> call_id;

                voips_->consume( simple_voip::create_drop_request( req_id, call_id ) );
            }
            else if( cmd == "play" )
            {
                auto req_id = req_id_gen_->get_next_request_id();

                uint32_t call_id;
                std::string filename;
                stream >> call_id >> filename;

                voips_->consume( simple_voip::wrap::create_PlayFileRequest( req_id, call_id, filename ) );
            }
            else if( cmd == "rec" )
            {
                auto req_id = req_id_gen_->get_next_request_id();

                uint32_t call_id;
                std::string filename;
                double duration;
                stream >> call_id >> filename >> duration;

                voips_->consume( simple_voip::wrap::create_RecordFileRequest( req_id, call_id, filename, duration ) );
            }
            else
                std::cout << "ERROR: unknown command '" << cmd << "'" << std::endl;
        }
        else
        {
            std::cout << "ERROR: cannot read command" << std::endl;
        }
        return true;
    }

private:
    simple_voip::ISimpleVoip    * voips_;
    utils::IRequestIdGen        * req_id_gen_;
};

int main( int argc, char **argv )
{
    dummy_logger::set_log_level( log_levels_log4j::DEBUG );

    simple_voip_dummy::Dummy        dialer;
    scheduler::Scheduler            sched( scheduler::Duration( std::chrono::milliseconds( 1 ) ) );
    simple_voip_wrap::Wrap          wrap;
    utils::RequestIdGen             req_id_gen;
    DurationGetter                  dg;
    Callback test;

    simple_voip_dummy::Config config;

    config_reader::ConfigReader cr;

    std::string config_file( "voip_config.ini" );

    if( cr.init( config_file ) == false )
    {
        std::cerr << "ERROR: cannot read config file " + config_file << std::endl;

        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "loaded config file " + config_file << std::endl;
    }

    simple_voip_dummy::init_config( & config, cr );

    std::string error_msg;

    auto log_id_wrap        = dummy_logger::register_module( "Wrap" );
    auto log_id_dummy       = dummy_logger::register_module( "SimpleVoipDummy" );
    auto log_id_call        = dummy_logger::register_module( "Call" );
    auto log_id_sched       = dummy_logger::register_module( "Scheduler" );

    dummy_logger::set_log_level( log_id_wrap,       log_levels_log4j::TRACE );
    dummy_logger::set_log_level( log_id_dummy,      log_levels_log4j::INFO );
    dummy_logger::set_log_level( log_id_call,       log_levels_log4j::TRACE );
    dummy_logger::set_log_level( log_id_sched,      log_levels_log4j::INFO );

    sched.init_log( log_id_sched );

    test.init( & wrap, & req_id_gen );

    bool b = dialer.init( log_id_dummy, log_id_call, config, & wrap, & sched, & error_msg );
    if( b == false )
    {
        std::cout << "cannot initialize voip module: " << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    {
        bool b = wrap.init( log_id_wrap, & dialer, & test, & sched, & req_id_gen, & dg, & error_msg );
        if( !b )
        {
            std::cout << "cannot initialize Wrap: " << error_msg << std::endl;
            return EXIT_FAILURE;
        }
    }

    dialer.start();

    std::vector< std::thread > tg;

    tg.push_back( std::thread( std::bind( &Callback::control_thread, &test ) ) );

    sched.run();

    for( auto & t : tg )
        t.join();

    dialer.shutdown();
    wrap.shutdown();
    sched.shutdown();

    std::cout << "Done! =)" << std::endl;

    return 0;
}
