if(EXISTS "/home/szzz/bustub/build_rel/test/binder_test[1]_tests.cmake")
  include("/home/szzz/bustub/build_rel/test/binder_test[1]_tests.cmake")
else()
  add_test(binder_test_NOT_BUILT binder_test_NOT_BUILT)
endif()
