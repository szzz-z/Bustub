if(EXISTS "/home/szzz/bustub/build_rel/test/txn_integration_test[1]_tests.cmake")
  include("/home/szzz/bustub/build_rel/test/txn_integration_test[1]_tests.cmake")
else()
  add_test(txn_integration_test_NOT_BUILT txn_integration_test_NOT_BUILT)
endif()
