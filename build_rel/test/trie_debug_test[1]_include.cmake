if(EXISTS "/home/szzz/bustub-2023spring/build_rel/test/trie_debug_test[1]_tests.cmake")
  include("/home/szzz/bustub-2023spring/build_rel/test/trie_debug_test[1]_tests.cmake")
else()
  add_test(trie_debug_test_NOT_BUILT trie_debug_test_NOT_BUILT)
endif()
