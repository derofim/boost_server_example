
# NOTE: INTERFACE library target does not directly create build output

add_library(Catch INTERFACE)
target_include_directories(Catch SYSTEM INTERFACE "Catch/single_include")
get_target_property (Catch_IMPORTED_LOCATION Catch INTERFACE_INCLUDE_DIRECTORIES)
message( "Catch=${Catch_IMPORTED_LOCATION}" )

add_library(FakeIt INTERFACE)
target_include_directories( FakeIt SYSTEM INTERFACE
  FakeIt/single_header
  FakeIt/include )
get_target_property (FakeIt_IMPORTED_LOCATION FakeIt INTERFACE_INCLUDE_DIRECTORIES)
message( "FakeIt=${FakeIt_IMPORTED_LOCATION}" )

add_library(boost_beast INTERFACE)
target_include_directories( boost_beast SYSTEM INTERFACE
  boost.beast/include )
get_target_property (BOOST_BEAST_IMPORTED_LOCATION boost_beast INTERFACE_INCLUDE_DIRECTORIES)
message( "Boost.beast=${BOOST_BEAST_IMPORTED_LOCATION}" )
