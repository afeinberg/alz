find_package(GTest)

set(COMMON_SOURCES 
  constants.h
  bit_stream.cc bit_stream.h
  sink_source.cc sink_source.h
  file_io.cc file_io.h
  decoder.cc decoder.h
  encoder.cc encoder.h)

add_executable(alz_encode encode_main.cc ${COMMON_SOURCES})
add_executable(alz_decode decode_main.cc ${COMMON_SOURCES})

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(bit_stream_test bit_stream_test.cc ${COMMON_SOURCES})
  add_executable(decoder_test decoder_test.cc ${COMMON_SOURCES})
  add_executable(encoder_test encoder_test.cc ${COMMON_SOURCES})
  add_executable(file_io_test file_io_test.cc ${COMMON_SOURCES})
  target_link_libraries(bit_stream_test ${GTEST_BOTH_LIBRARIES} pthread)  
  target_link_libraries(decoder_test ${GTEST_BOTH_LIBRARIES} pthread)
  target_link_libraries(encoder_test ${GTEST_BOTH_LIBRARIES} pthread)
  target_link_libraries(file_io_test ${GTEST_BOTH_LIBRARIES} pthread)
  add_test(bit_stream_test bit_stream_test)
  add_test(decoder_test decoder_test)
  add_test(encoder_test encoder_test)
endif()
