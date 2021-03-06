# This subproject implements integration tests.
# This tests are launched on the whole world dataset.

# It is recommended to place tests here in the following cases:
# - tests are written to be launch on the whole world dataset;
# - tests covers significant number of subsystems;

project(routing_integration_tests)

set(
  SRC
  bicycle_route_test.cpp
  bicycle_turn_test.cpp
  cross_section_tests.cpp
  get_altitude_test.cpp
  online_cross_tests.cpp
  pedestrian_route_test.cpp
  road_graph_tests.cpp
  route_test.cpp
  routing_test_tools.cpp
  routing_test_tools.hpp
  street_names_test.cpp
  turn_test.cpp
)

omim_add_test(${PROJECT_NAME} ${SRC})

omim_link_libraries(
  ${PROJECT_NAME}
  map
  routing
  search
  storage
  indexer
  editor
  traffic
  routing_common
  platform
  oauthcpp
  geometry
  coding
  base
  osrm
  jansson
  protobuf
  succinct
  stats_client
  pugixml
  opening_hours
  icu
  agg
  ${LIBZ}
)

link_qt5_core(${PROJECT_NAME})
