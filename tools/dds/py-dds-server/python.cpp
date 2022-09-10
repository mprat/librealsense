/* License: Apache 2.0. See LICENSE file in root directory.
Copyright(c) 2017 Intel Corporation. All Rights Reserved. */

#include "python.hpp"

#include <librealsense2/dds/dds-participant.h>
#include <librealsense2/utilities/easylogging/easyloggingpp.h>
//#include <librealsense2/rs.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastrtps/types/DynamicType.h>


INITIALIZE_EASYLOGGINGPP


namespace {
std::string to_string( librealsense::dds::dds_guid const & guid )
{
    std::ostringstream os;
    os << guid;
    return os.str();
}
}  // namespace


// "When calling a C++ function from Python, the GIL is always held"
// -- since we're not being called from Python but instead are calling it,
// we need to acquire it to not have issues with other threads...
#define FN_FWD( CLS, FN_NAME, PY_ARGS, FN_ARGS, CODE )                                                                 \
    #FN_NAME,                                                                                                          \
    []( CLS & self, std::function < void PY_ARGS > callback ) {                                                        \
        self.FN_NAME( [callback] FN_ARGS {                                                                             \
            try {                                                                                                      \
                py::gil_scoped_acquire gil;                                                                            \
                CODE                                                                                                   \
            }                                                                                                          \
            catch( ... ) {                                                                                             \
                std::cerr << "?!?!?!!? exception in python " #CLS "." #FN_NAME " ?!?!?!?!?" << std::endl;              \
            }                                                                                                          \
        } );                                                                                                           \
    }


PYBIND11_MODULE(NAME, m) {
    m.doc() = R"pbdoc(
        RealSense DDS Server Python Bindings
    )pbdoc";
    m.attr( "__version__" ) = "0.1";  // RS2_API_VERSION_STR;

    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally( el::ConfigurationType::ToStandardOutput, "true" );
    defaultConf.setGlobally( el::ConfigurationType::Format, " %datetime{%d/%M %H:%m:%s,%g} %level [%thread] (%fbase:%line) %msg" );
    el::Loggers::reconfigureLogger( "librealsense", defaultConf );

    m.def( "debug", []( bool enable = true ) {
        struct log_consumer : eprosima::fastdds::dds::LogConsumer
        {
            virtual void Consume( const eprosima::fastdds::dds::Log::Entry & e ) override
            {
                using eprosima::fastdds::dds::Log;
                switch( e.kind )
                {
                case Log::Kind::Error:
                    LOG_ERROR( "[DDS] " << e.message );
                    break;
                case Log::Kind::Warning:
                    LOG_WARNING( "[DDS] " << e.message );
                    break;
                case Log::Kind::Info:
                    LOG_DEBUG( "[DDS] " << e.message );
                    break;
                }
            }
        };

        if( enable )
        {
            // rs2_log_to_console( RS2_LOG_SEVERITY_DEBUG, nullptr );
            eprosima::fastdds::dds::Log::SetVerbosity( eprosima::fastdds::dds::Log::Info );
        }
        else
        {
            eprosima::fastdds::dds::Log::SetVerbosity( eprosima::fastdds::dds::Log::Error );
            // rs2::log_to_console( RS2_LOG_SEVERITY_ERROR );
        }
    });

    using librealsense::dds::dds_participant;
    using eprosima::fastrtps::types::ReturnCode_t;
    
    py::class_< dds_participant::listener,
                std::shared_ptr< dds_participant::listener >  // handled with a shared_ptr
                >
        listener( m, "participant.listener" );
    listener  // no ctor: use participant.create_listener()
        .def( FN_FWD( dds_participant::listener,
                      on_writer_added,
                      ( std::string const & guid, std::string const & topic_name ),
                      ( librealsense::dds::dds_guid guid, char const * topic_name ),
                      callback( to_string( guid ), topic_name ); ) )
        .def( FN_FWD( dds_participant::listener,
                      on_writer_removed,
                      ( std::string const & guid, std::string const & topic_name ),
                      ( librealsense::dds::dds_guid guid, char const * topic_name ),
                      callback( to_string( guid ), topic_name ); ) )
        .def( FN_FWD( dds_participant::listener,
                      on_reader_added,
                      ( std::string const & guid, std::string const & topic_name ),
                      ( librealsense::dds::dds_guid guid, char const * topic_name ),
                      callback( to_string( guid ), topic_name ); ) )
        .def( FN_FWD( dds_participant::listener,
                      on_reader_removed,
                      ( std::string const & guid, std::string const & topic_name ),
                      ( librealsense::dds::dds_guid guid, char const * topic_name ),
                      callback( to_string( guid ), topic_name ); ) )
        .def( FN_FWD( dds_participant::listener,
                      on_participant_added,
                      ( std::string const & guid, std::string const & name ),
                      ( librealsense::dds::dds_guid guid, char const * name ),
                      callback( to_string( guid ), name ); ) )
        .def( FN_FWD( dds_participant::listener,
                      on_participant_removed,
                      ( std::string const & guid, std::string const & name ),
                      ( librealsense::dds::dds_guid guid, char const * name ),
                      callback( to_string( guid ), name ); ) )
        .def( FN_FWD( dds_participant::listener,
                      on_type_discovery,
                      ( std::string const & topic_name, std::string const & type_name ),
                      ( char const * topic_name, eprosima::fastrtps::types::DynamicType_ptr dyn_type ),
                      callback( topic_name, dyn_type->get_name() ); ) );

    py::class_< dds_participant,
                std::shared_ptr< dds_participant >  // handled with a shared_ptr
                >
        participant( m, "participant" );
    participant.def( py::init<>() )
        .def( "init", &dds_participant::init, "domain-id"_a, "participant-name"_a )
        .def( "is_valid", &dds_participant::is_valid )
        .def( "__nonzero__", &dds_participant::is_valid ) // Called to implement truth value testing in Python 2
        .def( "__bool__", &dds_participant::is_valid )    // Called to implement truth value testing in Python 3
        .def( "__repr__",
              []( const dds_participant & self ) {
                  std::ostringstream os;
                  os << "<" SNAME ".dds_participant";
                  if( ! self.is_valid() )
                  {
                      os << " NULL";
                  }
                  else
                  {
                      eprosima::fastdds::dds::DomainParticipantQos qos;
                      if( ReturnCode_t::RETCODE_OK == self.get()->get_qos( qos ) )
                          os << " \"" << qos.name() << "\"";
                  }
                  os << ">";
                  return os.str();
            } )
        .def( "create_listener", []( dds_participant & self ) { return self.create_listener(); } );

}
