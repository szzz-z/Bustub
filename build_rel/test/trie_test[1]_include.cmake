if(EXISTS "/home/szzz/bustub/build_rel/test/trie_test[1]_tests.cmake")
  include("/home/szzz/bustub/build_rel/test/trie_test[1]_tests.cmake")
else()
  add_test(trie_test_NOT_BUILT trie_test_NOT_BUILT)
endif()
